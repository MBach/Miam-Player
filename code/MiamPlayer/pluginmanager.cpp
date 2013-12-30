#include "pluginmanager.h"

#include <QDirIterator>
#include <QLibrary>
#include <QMessageBox>
#include <QPluginLoader>

#include "mainwindow.h"
#include "settings.h"

/** Constructor with strong coupling. */
PluginManager::PluginManager(MainWindow *mainWindow) :
	QObject(mainWindow), _mainWindow(mainWindow)
{
	// Load or unload when a checkbox state has changed
	connect(_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget, &QTableWidget::itemChanged, this, &PluginManager::loadOrUnload);

	QDir appDirPath = QDir(qApp->applicationDirPath());
	if (appDirPath.cd("plugins")) {
		_pluginPath = appDirPath.absolutePath();
		this->init();
	}
}

/** Explicitly destroys every plugin. */
PluginManager::~PluginManager()
{
	QMapIterator<QString, BasicPluginInterface*> it(_instances);
	while (it.hasNext()) {
		delete it.next().value();
	}
}

/** Search into the subdir "plugins" where the application is installed.*/
void PluginManager::init()
{
	QDirIterator it(_pluginPath);
	Settings *settings = Settings::getInstance();
	while (it.hasNext()) {
		if (QLibrary::isLibrary(it.next())) {
			QString pluginFileName = it.fileName();
			QVariant vPluginInfo = settings->value(pluginFileName);
			// If it is the first time we trying to load the plugin
			if (vPluginInfo.isNull()) {
				this->loadPlugin(it.fileInfo());
			} else {
				PluginInfo pluginInfo = vPluginInfo.value<PluginInfo>();
				if (pluginInfo.isEnabled()) {
					this->loadPlugin(it.fileInfo());
				} else {
					// Plugin exists in Settings, but one has chosen to disable it at startup
					this->insertRow(pluginInfo);
					if (pluginInfo.isConfigurable()) {
						QWidget *fakeConfigPage = new QWidget();
						int tab = _mainWindow->customizeOptionsDialog->tabPlugins->addTab(fakeConfigPage, pluginInfo.pluginName());
						_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(tab, false);
					}
					_plugins.insert(pluginInfo.pluginName(), it.fileInfo());
				}
			}
		}
	}
	// No shared lib was found, so it is useless to keep a plugin page empty
	if (_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->rowCount() == 0) {
		_mainWindow->customizeOptionsDialog->listWidget->setRowHidden(5, true);
		_mainWindow->customizeOptionsDialog->stackedWidget->widget(5)->deleteLater();
	}
}

/** Insert a new row in the Plugin Page in Config Dialog with basic informations for each plugin. */
void PluginManager::insertRow(const PluginInfo &pluginInfo)
{
	// Add name, state and version info on a summary page
	int row = _mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->rowCount();
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->insertRow(row);
	QTableWidgetItem *checkbox = new QTableWidgetItem();
	if (pluginInfo.isEnabled()) {
		checkbox->setCheckState(Qt::Checked);
	} else {
		checkbox->setCheckState(Qt::Unchecked);
	}
	checkbox->setData(Qt::EditRole, QVariant::fromValue(pluginInfo));

	// Temporarily disconnects signals to prevent infinite recursion!
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->blockSignals(true);
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 0, new QTableWidgetItem(pluginInfo.pluginName()));
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 1, checkbox);
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 2, new QTableWidgetItem(pluginInfo.version()));
	_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->blockSignals(false);

	Settings::getInstance()->setValue(pluginInfo.fileName(), QVariant::fromValue(pluginInfo));
}

/** Load a plugin by its location on the hard drive. */
BasicPluginInterface * PluginManager::loadPlugin(const QFileInfo &pluginFileInfo)
{
	QPluginLoader pluginLoader(pluginFileInfo.absoluteFilePath(), this);
	QObject *plugin = pluginLoader.instance();
	if (plugin) {
		BasicPluginInterface *basic = dynamic_cast<BasicPluginInterface *>(plugin);
		if (basic) {
			// If one has previoulsy unloaded a plugin, and now wants to reload it (yeah, I know...), we don't need to append items once again
			///FIXME
			int idx = _mainWindow->customizeOptionsDialog->tabPlugins->count();
			if (_plugins.contains(basic->name())) {
				if (basic->isConfigurable()) {
					for (int i = 0; i < _mainWindow->customizeOptionsDialog->tabPlugins->count(); i++) {
						if (_mainWindow->customizeOptionsDialog->tabPlugins->tabText(i) == basic->name()) {
							_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(i, true);
							_mainWindow->customizeOptionsDialog->tabPlugins->widget(i)->deleteLater();
							idx = i;
							break;
						}
					}
				}
			} else {
				PluginInfo pluginInfo;
				pluginInfo.setFileName(pluginFileInfo.fileName());
				pluginInfo.setPluginName(basic->name());
				pluginInfo.setVersion(basic->version());
				pluginInfo.setConfigPage(basic->isConfigurable());
				pluginInfo.setEnabled(true);
				this->insertRow(pluginInfo);
				_plugins.insert(basic->name(), pluginFileInfo);
			}
			if (basic->isConfigurable()) {
				_mainWindow->customizeOptionsDialog->tabPlugins->insertTab(idx, basic->configPage(), basic->name());
			}
			// Keep references of loaded plugins, to be able to unload them later
			_instances.insert(basic->name(), basic);
		}

		/// XXX Make a dispatcher for other types of plugins?
		if (MediaPlayerPluginInterface *mediaPlayerPlugin = qobject_cast<MediaPlayerPluginInterface *>(plugin)) {
			qDebug() << "MediaPlayerPluginInterface" << mediaPlayerPlugin->name() << mediaPlayerPlugin->version();
			mediaPlayerPlugin->setMediaPlayer(_mainWindow->mediaPlayer());
			if (mediaPlayerPlugin->providesView()) {
				QAction *actionAddViewToMenu = new QAction(mediaPlayerPlugin->name(), _mainWindow->menuView);
				_mainWindow->menuView->addAction(actionAddViewToMenu);
				_mainWindow->updateFonts(Settings::getInstance()->font(Settings::MENUS));
				connect(actionAddViewToMenu, &QAction::triggered, [=]() {
					mediaPlayerPlugin->toggleViews(_mainWindow);
				});
				//QObjectList objects = { actionAddViewToMenu };
				QObjectList objects;
				objects.append(actionAddViewToMenu);
				_dependencies.insert(basic->name(), objects);
			}
		} /// else...
		return basic;
	} else {
		QString message = QString(tr("A plugin was found but was the player was unable to load it (file %1)").arg(pluginFileInfo.fileName()));
		QMessageBox *m = new QMessageBox(QMessageBox::Warning, "Warning", message, QMessageBox::Close, _mainWindow);
		m->show();
	}
	return NULL;
}

/** Unload a plugin by its name. */
void PluginManager::unloadPlugin(const QString &pluginName)
{
	BasicPluginInterface *basic = _instances.value(pluginName);
	foreach (QObject *dependency, _dependencies.value(pluginName)) {
		if (QAction *action = qobject_cast<QAction*>(dependency)) {
			QMenu *menu = qobject_cast<QMenu*>(action->parent());
			menu->removeAction(action);
		} /// else...
	}
	// Search and disable the config page in options page
	if (basic->isConfigurable()) {
		for (int i = 0; i < _mainWindow->customizeOptionsDialog->tabPlugins->count(); i++) {
			if (_mainWindow->customizeOptionsDialog->tabPlugins->tabText(i) == basic->name()) {
				_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(i, false);
				break;
			}
		}
	}
	_instances.remove(pluginName);
	_dependencies.remove(pluginName);
	delete basic;
	basic = NULL;
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
			this->loadPlugin(fileInfo);
		} else {
			this->unloadPlugin(pluginInfo.pluginName());
		}
		// Keep in settings if the plugin is enabled. Useful when starting the application for unwanted plugins
		pluginInfo.setEnabled(item->checkState() == Qt::Checked);
		Settings::getInstance()->setValue(pluginInfo.fileName(), QVariant::fromValue(pluginInfo));
	}
}
