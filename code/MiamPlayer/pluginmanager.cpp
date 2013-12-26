#include "pluginmanager.h"

#include <QDirIterator>
#include <QLibrary>
#include <QMessageBox>
#include <QPluginLoader>

#include "mainwindow.h"

PluginManager::PluginManager(MainWindow *mainWindow) :
	QObject(mainWindow), _mainWindow(mainWindow)
{
	// Load or unload plugins dynamically
	connect(_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget, &QTableWidget::itemChanged, this, &PluginManager::loadOrUnload);
}

void PluginManager::init(const QDir &appDirPath)
{
	QDirIterator it(appDirPath);
	while (it.hasNext()) {
		if (QLibrary::isLibrary(it.next())) {
			this->loadPlugin(it.fileInfo());
		}
	}
	if (_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->rowCount() == 0) {
		_mainWindow->customizeOptionsDialog->listWidget->setRowHidden(5, true);
	}
}

void PluginManager::loadPlugin(const QFileInfo &pluginFileInfo)
{
	QPluginLoader pluginLoader(pluginFileInfo.absoluteFilePath(), this);
	QObject *plugin = pluginLoader.instance();
	if (plugin) {
		BasicPluginInterface *basic = dynamic_cast<BasicPluginInterface *>(plugin);
		if (basic) {
			// If one has previoulsy unloaded a plugin, and now wants to reload it (yeah, I know...), we don't need to append items once again
			if (_plugins.contains(basic->name())) {
				for (int i = 0; i < _mainWindow->customizeOptionsDialog->tabPlugins->count(); i++) {
					if (_mainWindow->customizeOptionsDialog->tabPlugins->tabText(i) == basic->name()) {
						_mainWindow->customizeOptionsDialog->tabPlugins->setTabEnabled(i, true);
						break;
					}
				}
			} else {
				// Attach a new config page it the plugin provides one
				if (basic->configPage()) {
					_mainWindow->customizeOptionsDialog->tabPlugins->addTab(basic->configPage(), basic->name());
				}

				// Add name, state and version info on a summary page
				int row = _mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->rowCount();
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->insertRow(row);
				QTableWidgetItem *checkbox = new QTableWidgetItem();
				checkbox->setCheckState(Qt::Checked);

				// Temporarily disconnects signals to prevent infinite recursion!
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->blockSignals(true);
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 0, new QTableWidgetItem(basic->name()));
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 1, checkbox);
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->setItem(row, 2, new QTableWidgetItem(basic->version()));
				_mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->blockSignals(false);

				_plugins.insert(basic->name(), pluginFileInfo);
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
				QObjectList objects = { actionAddViewToMenu };
				_dependencies.insert(basic->name(), objects);
			}
		} /// else...
	} else {
		QString message = QString(tr("A plugin was found but was the player was unable to load it (file %1)")).arg(pluginFileInfo.fileName());
		QMessageBox *m = new QMessageBox(QMessageBox::Warning, "Warning", message, QMessageBox::Close, _mainWindow);
		m->show();
	}
}

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
	if (basic->configPage()) {
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

void PluginManager::loadOrUnload(QTableWidgetItem *item)
{
	if (item->column() == 1) {
		QString name = _mainWindow->customizeOptionsDialog->pluginSummaryTableWidget->item(item->row(), 0)->text();
		if (item->checkState() == Qt::Checked) {
			this->loadPlugin(_plugins.value(name));
		} else {
			this->unloadPlugin(name);
		}
	}
}
