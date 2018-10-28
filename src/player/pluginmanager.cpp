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
{}

/** Explicitly destroys every plugin. */
PluginManager::~PluginManager()
{
	qDeleteAll(_loadedPlugins);
	_loadedPlugins.clear();
	qDeleteAll(_dependencies);
	_dependencies.clear();

	/*QMapIterator<QString, BasicPlugin*> it(_loadedPlugins);
	while (it.hasNext()) {
		it.next();
		this->unloadPlugin(it.key());
	}*/
}

void PluginManager::init()
{
	QDir appDirPath = QDir(qApp->applicationDirPath());
	QString pluginPath;
#if defined(Q_OS_OSX)
	appDirPath.cdUp();
	appDirPath.cd("PlugIns");
	pluginPath = appDirPath.absolutePath();
#else
	if (appDirPath.cd("plugins")) {
		pluginPath = appDirPath.absolutePath();
	}
#endif
	if (pluginPath.isEmpty()) {
		return;
	}
	QDirIterator it(pluginPath);
	SettingsPrivate *settings = SettingsPrivate::instance();
	QMap<QString, PluginInfo> plugins = settings->plugins();
	QStringList failedPlugins;
	while (it.hasNext()) {
		it.next();
		if (QLibrary::isLibrary(it.fileName()) && !it.fileInfo().isSymLink()) {
			// If plugin was recognized by the App at least once
			if (plugins.contains(it.filePath())) {
				PluginInfo pluginInfo = plugins.value(it.filePath());
				if (pluginInfo.isEnabled() && !this->loadPlugin(it.filePath())) {
					failedPlugins << it.fileName();
				}
			} else if (!this->loadPlugin(it.filePath())) {
				failedPlugins << it.fileName();
			}
		}
	}

	// If at least one plugin wasn't restored (API has changed for example)
	if (!failedPlugins.isEmpty()) {
		this->alertUser(failedPlugins);
	}
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
bool PluginManager::loadPlugin(const QString &pluginAbsPath)
{
	QPluginLoader pluginLoader(pluginAbsPath, this);
	QObject *plugin = pluginLoader.instance();
	if (plugin) {
		BasicPlugin *basic = static_cast<BasicPlugin*>(plugin);
		if (basic) {
			basic->setParent(this);
			PluginInfo pluginInfo;
			pluginInfo.setAbsFilePath(pluginAbsPath);
			pluginInfo.setPluginName(basic->name());
			pluginInfo.setVersion(basic->version());
			pluginInfo.setConfigPage(basic->isConfigurable());
			pluginInfo.setEnabled(true);

			SettingsPrivate *settings = SettingsPrivate::instance();
			settings->addPlugin(pluginInfo);
			if (basic->isConfigurable()) {
				QString pluginLang(":/translations/" + basic->name() + "_" + settings->language());
				if (basic->translator.load(pluginLang)) {
					QApplication::installTranslator(&basic->translator);
				}
			}

			// Keep references of loaded plugins, to be able to unload them later
			_loadedPlugins.insert(pluginAbsPath, basic);
		} else {
			return false;
		}

		basic->init();
		if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
			this->loadMediaPlayerPlugin(mediaPlayerPlugin);
		} else if (RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin = qobject_cast<RemoteMediaPlayerPlugin*>(plugin)) {
			this->loadRemoteMediaPlayerPlugin(remoteMediaPlayerPlugin);
        }
	}
	return plugin != nullptr;
}

/** Allow views to be extended by adding 1 or more entries in a context menu and items to interact with. */
void PluginManager::registerExtensionPoint(QPair<QString, QObjectList> ext)
{
	for (QObject *instance : ext.second) {
		_extensionPoints.insert(ext.first, instance);
	}

	// Reload views
	for (BasicPlugin *plugin : _loadedPlugins.values()) {
		if (!plugin) {
			qWarning() << Q_FUNC_INFO << "Plugin shouldn't be null";
			continue;
		}
		if (ItemViewPlugin *itemViewPlugin = qobject_cast<ItemViewPlugin*>(plugin)) {
			this->loadItemViewPlugin(itemViewPlugin);
		}/* else if (TagEditorPlugin *tagEditorPlugin = qobject_cast<TagEditorPlugin*>(plugin)) {
			this->loadTagEditorPlugin(tagEditorPlugin);
		}*/
	}
}

void PluginManager::unregisterExtensionPoint(const QString &ext)
{
	for (QObject *instance : _extensionPoints.values(ext)) {
		if (instance) {
			instance->deleteLater();
		}
	}
	_extensionPoints.remove(ext);
}

/** Unload a plugin by its name. */
bool PluginManager::unloadPlugin(const QString &absFilePath)
{
	BasicPlugin *basic = _loadedPlugins.value(absFilePath);
	for (QObject *dependency : _dependencies.values(basic->name())) {
		if (QAction *action = qobject_cast<QAction*>(dependency)) {
			QMenu *menu = qobject_cast<QMenu*>(action->parent());
			menu->removeAction(action);
		} else if (QMenu *menu = qobject_cast<QMenu*>(dependency)) {
			menu->deleteLater();
		}
	}

	_loadedPlugins.remove(absFilePath);
	_dependencies.remove(basic->name());
	basic->cleanUpBeforeDestroy();
	delete basic;
	basic = nullptr;
	auto settings = SettingsPrivate::instance();
	settings->disablePlugin(absFilePath);
	return true;
}

void PluginManager::loadItemViewPlugin(ItemViewPlugin *itemViewPlugin)
{
	// Each View Plugin can extend multiple instances
	/*for (QString view : itemViewPlugin->classesToExtend()) {

		// Instances of classes which can be extended at runtime
		for (QObject *instance : _extensionPoints.values(view)) {

			// QMenu and SelectedTracksModel are the 2 kinds of class which can be extended
			if (QMenu *menu = qobject_cast<QMenu*>(instance)) {
				if (itemViewPlugin->hasSubMenu(view)) {
					QMenu *subMenu = itemViewPlugin->menu(view, menu);
					menu->addMenu(subMenu);
					_dependencies.insert(itemViewPlugin->name(), subMenu);
				} else {
					QAction *action = itemViewPlugin->action(view, menu);
					menu->addAction(action);
					_dependencies.insert(itemViewPlugin->name(), action);
				}
			} else if (SelectedTracksModel *selectedTracksModel = dynamic_cast<SelectedTracksModel*>(instance)) {
				itemViewPlugin->setSelectedTracksModel(view, selectedTracksModel);
			}
		}
	}*/
}

void PluginManager::loadMediaPlayerPlugin(MediaPlayerPlugin *mediaPlayerPlugin)
{
	if (mediaPlayerPlugin->hasView()) {
		QAction *actionAddViewToMenu = new QAction(mediaPlayerPlugin->name(), _mainWindow->menuView);
		actionAddViewToMenu->setObjectName(mediaPlayerPlugin->name());
		_mainWindow->menuView->insertAction(_mainWindow->actionViewTagEditor, actionAddViewToMenu);
		_mainWindow->updateFonts(SettingsPrivate::instance()->font(SettingsPrivate::FF_Menu));

		// Link the view to the existing ActionGroup
		actionAddViewToMenu->setCheckable(true);
		actionAddViewToMenu->setActionGroup(_mainWindow->actionViewPlaylists->actionGroup());
		_dependencies.insert(mediaPlayerPlugin->name(), actionAddViewToMenu);
	}
}

void PluginManager::loadRemoteMediaPlayerPlugin(RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin)
{
	qDebug() << Q_FUNC_INFO << remoteMediaPlayerPlugin->player();
	_mainWindow->mediaPlayer()->addRemotePlayer(remoteMediaPlayerPlugin->player());
}

/*
void PluginManager::loadTagEditorPlugin(TagEditorPlugin *tagEditorPlugin)
{
	tagEditorPlugin->setLocalPlayer(_mainWindow->mediaPlayer()->localPlayer());
	qDebug() << Q_FUNC_INFO << tagEditorPlugin <<  _extensionPoints.keys();

	// Instances of classes which can be extended at runtime
	for (QObject *instance : _extensionPoints.values("TagEditor")) {
		if (TagEditor *tagEditor = qobject_cast<TagEditor*>(instance)) {
			tagEditorPlugin->setSelectedTracksModel(tagEditor);
		} else if (QStackedWidget *extensibleWidgetArea = qobject_cast<QStackedWidget*>(instance)){
			tagEditorPlugin->setStackWidget(extensibleWidgetArea);
		} else if (QHBoxLayout *hbox = qobject_cast<QHBoxLayout*>(instance)) {
			tagEditorPlugin->setExtensibleLayout(hbox);
		} else if (QTableWidget *table = qobject_cast<QTableWidget*>(instance)) {
			tagEditorPlugin->setTagEditorWidget(table);
		}
	}
}
*/
