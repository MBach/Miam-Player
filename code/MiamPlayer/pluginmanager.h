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
 * \version     1.0
 * \copyright   GNU General Public License v3
 */
class PluginManager : public QObject
{
	Q_OBJECT
private:
	MainWindow *_mainWindow;
	QMap<QString, QFileInfo> _plugins;
	QMap<QString, BasicPluginInterface*> _instances;
	QMap<QString, QObjectList> _dependencies;

public:
	/** Constructor with strong coupling. */
	explicit PluginManager(MainWindow *mainWindow);

	/** Explicitly destroys every plugin. */
	virtual ~PluginManager();

	/** Search into the subdir "plugins" where the application is installed.*/
	void init(const QDir &appDirPath);

private:
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
