#include "pluginmanager.h"

#include <QDirIterator>
#include <QLibrary>
#include <QMessageBox>
#include <QPluginLoader>

#include <settings.h>
#include "mainwindow.h"
#include "settingsprivate.h"
#include "model/selectedtracksmodel.h"
#include "abstractsearchdialog.h"

/** Constructor with strong coupling. */
PluginManager::PluginManager(MainWindow *parent)
	: QObject(parent)
	, _mainWindow(parent)
{
	QDir appDirPath = QDir(qApp->applicationDirPath());
	if (appDirPath.cd("plugins")) {
		_pluginPath = appDirPath.absolutePath();
		QDirIterator it(_pluginPath);
		SettingsPrivate *settings = SettingsPrivate::instance();
		QMap<QString, PluginInfo> plugins = settings->plugins();
		QStringList failedPlugins;
		while (it.hasNext()) {
			if (QLibrary::isLibrary(it.next()) && !it.fileInfo().isSymLink()) {
				// If plugin was recognized by the App at least once
				if (plugins.contains(it.fileName())) {
					PluginInfo pluginInfo = plugins.value(it.fileName());
					if (pluginInfo.isEnabled() && !this->loadPlugin(it.fileName())) {
						failedPlugins << it.fileName();
					}
				} else if (!this->loadPlugin(it.fileName())) {
					failedPlugins << it.fileName();
				}
			}
		}

		// If at least one plugin wasn't restored (API has changed for example)
		if (!failedPlugins.isEmpty()) {
			this->alertUser(failedPlugins);
		}
	}
}

/** Explicitly destroys every plugin. */
PluginManager::~PluginManager()
{
	qDeleteAll(_loadedPlugins);
	_loadedPlugins.clear();
}

/** Display a QMessageBox if at least one error was encountered when loading plugins. */
void PluginManager::alertUser(const QStringList &failedPlugins)
{
	QMessageBox *m = new QMessageBox(_mainWindow);
	m->setWindowTitle(tr("Warning"));
	m->setIcon(QMessageBox::Warning);
	if (failedPlugins.size() == 1) {
		m->setText(tr("A plugin was found but was the player was unable to load it (file %1)").arg(failedPlugins.first()));
	} else {
		m->setText(tr("Some plugins were found but the player was unable to load them.\n\nThe API has changed and you need to update these plugins too!"));
		QString detailedMsg = tr("Incompatible plugins:\n");
		for (QString f : failedPlugins) {
			detailedMsg.append(f + "\n");
		}
		m->setDetailedText(detailedMsg);
	}
	m->show();
}

/** Load a plugin by its location on the hard drive. */
bool PluginManager::loadPlugin(const QString &fileName)
{
	QString pluginAbsPath = QDir::toNativeSeparators(_pluginPath + "/" + fileName);
	QPluginLoader pluginLoader(pluginAbsPath, this);
	QObject *plugin = pluginLoader.instance();
	if (plugin) {
		BasicPlugin *basic = dynamic_cast<BasicPlugin*>(plugin);
		SettingsPrivate *settings = SettingsPrivate::instance();
		if (basic) {
			PluginInfo pluginInfo;
			pluginInfo.setFileName(fileName);
			pluginInfo.setPluginName(basic->name());
			pluginInfo.setVersion(basic->version());
			pluginInfo.setConfigPage(basic->isConfigurable());
			pluginInfo.setEnabled(true);
			qDebug() << Q_FUNC_INFO << pluginInfo.pluginName() << "has been loaded";

			settings->addPlugin(pluginInfo);
			if (basic->isConfigurable()) {
				QString pluginLang(":/translations/" + basic->name() + "_" + settings->language());
				if (basic->translator.load(pluginLang)) {
					QApplication::installTranslator(&basic->translator);
				}
			}

			// Keep references of loaded plugins, to be able to unload them later
			_loadedPlugins.insert(fileName, basic);
		} else {
			return false;
		}

		if (SearchMediaPlayerPlugin *searchMediaPlayerPlugin = qobject_cast<SearchMediaPlayerPlugin*>(plugin)) {
			this->loadSearchMediaPlayerPlugin(searchMediaPlayerPlugin);
		} else if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
			this->loadMediaPlayerPlugin(mediaPlayerPlugin);
		} else if (ItemViewPlugin *itemViewPlugin = qobject_cast<ItemViewPlugin*>(plugin)) {
			this->loadItemViewPlugin(itemViewPlugin);
		} else if (RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin = qobject_cast<RemoteMediaPlayerPlugin*>(plugin)) {
			this->loadRemoteMediaPlayerPlugin(remoteMediaPlayerPlugin);
		} else if (TagEditorPlugin *tagEditorPlugin = qobject_cast<TagEditorPlugin*>(plugin)) {
			this->loadTagEditorPlugin(tagEditorPlugin);
		}
		basic->init();
	}
	return plugin != nullptr;
}

/** Allow views to be extended by adding 1 or more entries in a context menu and items to interact with. */
/*void PluginManager::registerExtensionPoint(const char *className, QObjectList source)
{
	for (QObject *object : source) {
		_extensionPoints.insert(QString(className), object);
	}
}*/

/** Unload a plugin by its name. */
bool PluginManager::unloadPlugin(const QString &fileName)
{
	BasicPlugin *basic = _loadedPlugins.value(fileName);
	for (QObject *dependency : _dependencies.values(basic->name())) {
		if (QAction *action = qobject_cast<QAction*>(dependency)) {
			QMenu *menu = qobject_cast<QMenu*>(action->parent());
			menu->removeAction(action);
		} else if (QMenu *menu = qobject_cast<QMenu*>(dependency)) {
			delete menu;
		}
	}

	_loadedPlugins.remove(fileName);
	_dependencies.remove(basic->name());
	basic->cleanUpBeforeDestroy();
	delete basic;
	basic = nullptr;
	auto settings = SettingsPrivate::instance();
	settings->disablePlugin(fileName);
	qDebug() << Q_FUNC_INFO << fileName << "has been unloaded";
	return true;
}

void PluginManager::loadItemViewPlugin(ItemViewPlugin *itemViewPlugin)
{
	// Each View Plugin can extend multiple instances
	qDebug() << Q_FUNC_INFO << itemViewPlugin->classesToExtend();

	QMultiMap<QString, QObject*> extensionPoints;// = Settings::instance()->extensionPoints();

	for (QString view : itemViewPlugin->classesToExtend()) {

		qDebug() << "trying to extend view:" << view;


		// Instances of classes which can be extended at runtime
		for (QObject *obj : extensionPoints.values(view)) {

			qDebug() << "extension point:" << obj->objectName() << obj;

			// QMenu and SelectedTracksModel are the 2 kinds of class which can be extended
			if (QMenu *menu = qobject_cast<QMenu*>(obj)) {
				if (itemViewPlugin->hasSubMenu(view)) {
					QMenu *subMenu = itemViewPlugin->menu(view, menu);
					menu->addMenu(subMenu);
					_dependencies.insert(itemViewPlugin->name(), subMenu);
				} else {
					QAction *action = itemViewPlugin->action(view, menu);
					menu->addAction(action);
					_dependencies.insert(itemViewPlugin->name(), action);
				}
			} else if (SelectedTracksModel *selectedTracksModel = dynamic_cast<SelectedTracksModel*>(obj)) {
				itemViewPlugin->setSelectedTracksModel(view, selectedTracksModel);
			}
		}
	}
}

void PluginManager::loadMediaPlayerPlugin(MediaPlayerPlugin *mediaPlayerPlugin)
{
	QWidget *view = mediaPlayerPlugin->providesView();
	if (view != nullptr) {
		// Add a separator before any plugin (3 views by default: Playlist, Unique Library and Tag Editor
		if (_mainWindow->menuView->actions().count() == 3) {
			_mainWindow->menuView->addSeparator();
		}
		QAction *actionAddViewToMenu = new QAction(mediaPlayerPlugin->name(), _mainWindow->menuView);
		actionAddViewToMenu->setObjectName(mediaPlayerPlugin->name());
		_mainWindow->menuView->addAction(actionAddViewToMenu);
		_mainWindow->updateFonts(SettingsPrivate::instance()->font(SettingsPrivate::FF_Menu));
		connect(actionAddViewToMenu, &QAction::triggered, this, [=]() {
			_mainWindow->close();
			view->show();
		});

		// Link the view to the existing ActionGroup
		actionAddViewToMenu->setCheckable(true);
		actionAddViewToMenu->setActionGroup(_mainWindow->actionViewPlaylists->actionGroup());
		_dependencies.insert(mediaPlayerPlugin->name(), actionAddViewToMenu);
	}
	mediaPlayerPlugin->setMediaPlayer(_mainWindow->mediaPlayer());
}

void PluginManager::loadRemoteMediaPlayerPlugin(RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin)
{
	remoteMediaPlayerPlugin->setSearchDialog(_mainWindow->searchDialog);
	_mainWindow->mediaPlayer()->addRemotePlayer(remoteMediaPlayerPlugin->player());
}

void PluginManager::loadSearchMediaPlayerPlugin(SearchMediaPlayerPlugin *searchMediaPlayerPlugin)
{
	searchMediaPlayerPlugin->setSearchDialog(_mainWindow->searchDialog);
}

void PluginManager::loadTagEditorPlugin(TagEditorPlugin *tagEditorPlugin)
{
	tagEditorPlugin->setSelectedTracksModel(_mainWindow->tagEditor);
	tagEditorPlugin->setStackWidget(_mainWindow->tagEditor->extensibleWidgetArea);
	tagEditorPlugin->setExtensibleLayout(_mainWindow->tagEditor->extensiblePushButtonArea);
	tagEditorPlugin->setTagEditorWidget(_mainWindow->tagEditor->tagEditorWidget);
}
