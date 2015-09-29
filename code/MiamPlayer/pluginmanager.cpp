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
		QStringList failedPlugins;
		while (it.hasNext()) {
			if (QLibrary::isLibrary(it.next()) && !it.fileInfo().isSymLink()) {
				QVariant vPluginInfo = settings->value(it.fileName());
				// If it is the first time we trying to load the plugin
				if (vPluginInfo.isNull()) {
					if (!this->loadPlugin(it.fileInfo())) {
						failedPlugins << it.fileName();
					}
				} else {
					PluginInfo pluginInfo = vPluginInfo.value<PluginInfo>();
					if (pluginInfo.isEnabled()) {
						if (!this->loadPlugin(it.fileInfo())) {
							failedPlugins << it.fileName();
						}
					} else {
						// Plugin exists in Settings, but one has chosen to disable it at startup
						//this->insertRow(pluginInfo);
						if (pluginInfo.isConfigurable()) {
							//QWidget *fakeConfigPage = new QWidget();
							emit createDisabledPluginTab(pluginInfo.pluginName());
							//int tab = _mainWindow->customizeOptionsDialog->tabPlugins->addTab(fakeConfigPage, pluginInfo.pluginName());
							//_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(tab, false);
							//_mainWindow->customizeOptionsDialog->tabPlugins->setTabToolTip(tab, tr("You have chosen to disable this plugin, therefore you cannot access to its configuration page right now."));
						}
						_plugins.insert(pluginInfo.pluginName(), it.fileInfo());
					}
				}
			}
		}

		// If at least one plugin wasn't restored (API has changed for example)
		if (!failedPlugins.isEmpty()) {
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
	}
}

/** Explicitly destroys every plugin. */
PluginManager::~PluginManager()
{
	qDeleteAll(_instances);
	_instances.clear();
}

/** Allow views to be extended by adding 1 or more entries in a context menu and items to interact with. */
void PluginManager::registerExtensionPoint(const char *className, QObjectList source)
{
	for (QObject *object : source) {
		_extensionPoints.insert(QString(className), object);
	}
}

void PluginManager::loadItemViewPlugin(ItemViewPlugin *itemViewPlugin)
{
	// Each View Plugin can extend multiple instances
	for (QString view : itemViewPlugin->classesToExtend()) {

		// Instances of classes which can be extended at runtime
		for (QObject *obj : _extensionPoints.values(view)) {
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

/** Load a plugin by its location on the hard drive. */
bool PluginManager::loadPlugin(const QFileInfo &pluginFileInfo)
{
	QPluginLoader pluginLoader(pluginFileInfo.absoluteFilePath(), this);
	QObject *plugin = pluginLoader.instance();
	if (plugin) {
		qDebug() << Q_FUNC_INFO << pluginFileInfo.absoluteFilePath() << "has been loaded";
		BasicPlugin *basic = dynamic_cast<BasicPlugin*>(plugin);
		if (basic) {
			SettingsPrivate *settings = SettingsPrivate::instance();
			// If one has previoulsy unloaded a plugin, and now wants to reload it (yeah, I know...), we don't need to append items once again
			///FIXME
			//int idx = _mainWindow->customizeOptionsDialog->tabPlugins->count();
			if (_plugins.contains(basic->name())) {
				if (basic->isConfigurable()) {
					/*for (int i = 0; i < _mainWindow->customizeOptionsDialog->tabPlugins->count(); i++) {
						if (_mainWindow->customizeOptionsDialog->tabPlugins->tabText(i) == basic->name()) {
							_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(i, true);
							_mainWindow->customizeOptionsDialog->tabPlugins->widget(i)->deleteLater();
							idx = i;
							break;
						}
					}*/
				}
			} else {
				PluginInfo pluginInfo;
				pluginInfo.setFileName(pluginFileInfo.fileName());
				pluginInfo.setPluginName(basic->name());
				pluginInfo.setVersion(basic->version());
				pluginInfo.setConfigPage(basic->isConfigurable());
				pluginInfo.setEnabled(true);
				//this->insertRow(pluginInfo);
				_plugins.insert(basic->name(), pluginFileInfo);
			}
			if (basic->isConfigurable()) {
				QString pluginLang(":/translations/" + basic->name() + "_" + settings->language());
				if (basic->translator.load(pluginLang)) {
					QApplication::installTranslator(&basic->translator);
				}
				//_mainWindow->customizeOptionsDialog->tabPlugins->insertTab(idx, basic->configPage(), basic->name());
			}
			// Keep references of loaded plugins, to be able to unload them later
			_instances.insert(basic->name(), basic);
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

/** Unload a plugin by its name. */
void PluginManager::unloadPlugin(const QString &pluginName)
{
	BasicPlugin *basic = _instances.value(pluginName);
	for (QObject *dependency : _dependencies.values(pluginName)) {
		if (QAction *action = qobject_cast<QAction*>(dependency)) {
			QMenu *menu = qobject_cast<QMenu*>(action->parent());
			menu->removeAction(action);
		} else if (QMenu *menu = qobject_cast<QMenu*>(dependency)) {
			delete menu;
		}
	}
	// Search and disable the config page in options page
	if (basic->isConfigurable()) {
		/// FIXME
		/*for (int i = 0; i < _mainWindow->customizeOptionsDialog->tabPlugins->count(); i++) {
			if (_mainWindow->customizeOptionsDialog->tabPlugins->tabText(i) == basic->name()) {
				_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(i, false);
				break;
			}
		}*/
	}
	_instances.remove(pluginName);
	_dependencies.remove(pluginName);
	basic->cleanUpBeforeDestroy();
	delete basic;
	basic = nullptr;
}

/** Load or unload a plugin when one is switching a checkbox in the options. */
void PluginManager::loadOrUnload(QTableWidgetItem *item)
{
	// Checkboxes are in the second column
	if (item->column() == 1) {
		QVariant vPluginInfo = item->data(Qt::EditRole);
		PluginInfo pluginInfo = vPluginInfo.value<PluginInfo>();
		if (item->checkState() == Qt::Checked) {
			QString pluginAbsPath = QDir::toNativeSeparators(_pluginPath + "/" + pluginInfo.fileName());
			QFileInfo fileInfo(pluginAbsPath);
			bool ok = this->loadPlugin(fileInfo);
			if (!ok) {

			}
		} else {
			this->unloadPlugin(pluginInfo.pluginName());
		}
		// Keep in settings if the plugin is enabled. Useful when starting the application for unwanted plugins
		pluginInfo.setEnabled(item->checkState() == Qt::Checked);
		SettingsPrivate::instance()->setValue(pluginInfo.fileName(), QVariant::fromValue(pluginInfo));
	}
}
