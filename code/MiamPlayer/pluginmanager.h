#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <interfaces/mediaplayerplugininterface.h>
#include <QDir>

#include <QTableWidgetItem>

#include "plugininfo.h"

class MainWindow;

/**
 * \brief		The PluginManager class can dynamically load or unload plugins without restarting the application
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class PluginManager : public QObject
{
	Q_OBJECT
private:
	/** Reference to the MainWindow instance. */
	MainWindow *_mainWindow;

	/** Every plugin can be located on the hard drive by its complete filename. */
	QMap<QString, QFileInfo> _plugins;

	/** Loaded plugins are stored in this map. */
	QMap<QString, BasicPluginInterface*> _instances;

	/** Every plugin might instanciate objects that we need to be able to delete later (especially for unloading). */
	QMap<QString, QObjectList> _dependencies;

	/** Plugins are stored in a subdirectory called "plugins" under the application path. */
	QString _pluginPath;

public:
	/** Constructor with strong coupling. */
	explicit PluginManager(MainWindow *mainWindow);

	/** Explicitly destroys every plugin. */
	virtual ~PluginManager();

private:
	/** Search into the subdir "plugins" where the application is installed.*/
	void init();

	/** Insert a new row in the Plugin Page in Config Dialog with basic informations for each plugin. */
	void insertRow(const PluginInfo &pluginInfo);

	/** Load a plugin by its location on the hard drive. */
	BasicPluginInterface *loadPlugin(const QFileInfo &pluginFileInfo);

	/** Unload a plugin by its name. */
	void unloadPlugin(const QString &pluginName);

private slots:
	/** Load or unload a plugin when one is switching a checkbox in the options. */
	void loadOrUnload(QTableWidgetItem *item);
};

#endif // PLUGINMANAGER_H
