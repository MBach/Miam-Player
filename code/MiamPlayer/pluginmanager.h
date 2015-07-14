#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <interfaces/itemviewplugin.h>
#include <interfaces/searchmediaplayerplugin.h>
#include <interfaces/remotemediaplayerplugin.h>
#include <interfaces/tageditorplugin.h>

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
	/** The unique instance of this class. */
	static PluginManager *_pluginManager;

	/** Reference to the MainWindow instance (strong coupling). */
	MainWindow *_mainWindow;

	/** Every plugin can be located on the hard drive by its complete filename. */
	QMap<QString, QFileInfo> _plugins;

	/** Loaded plugins are stored in this map. */
	QMap<QString, BasicPlugin*> _instances;

	/** Every plugin might instanciate objects that we need to be able to delete later (especially for unloading). */
	QMultiMap<QString, QObject*> _dependencies;

	/** Plugins are stored in a subdirectory called "plugins" under the application path. */
	QString _pluginPath;

	/** Some instances in the software can be modified (menus, buttons, widgets, etc). */
	QMultiMap<QString, QObject*> _extensionPoints;

	PluginManager(QObject *parent = 0);

public:
	/** Singleton pattern to be able to easily use this plugin manager everywhere in the app. */
	static PluginManager* instance();

	void setMainWindow(MainWindow *mainWindow);

	/** Explicitly destroys every plugin. */
	virtual ~PluginManager();

	/** Allow views to be extended by adding 1 or more entries in a context menu and items to interact with. */
	void registerExtensionPoint(const char *className, QObjectList target);

	inline QList<BasicPlugin*> plugins() const { return _instances.values(); }

private:
	/** Search into the subdir "plugins" where the application is installed. */
	void init();

	/** Insert a new row in the Plugin Page in Config Dialog with basic informations for each plugin. */
	void insertRow(const PluginInfo &pluginInfo);

	/** Load a plugin by its location on the hard drive. */
	BasicPlugin *loadPlugin(const QFileInfo &pluginFileInfo);

	void loadItemViewPlugin(ItemViewPlugin *itemViewPlugin);
	void loadMediaPlayerPlugin(MediaPlayerPlugin *mediaPlayerPlugin);
	void loadRemoteMediaPlayerPlugin(RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin);
	void loadSearchMediaPlayerPlugin(SearchMediaPlayerPlugin *searchMediaPlayerPlugin);
	void loadTagEditorPlugin(TagEditorPlugin *tagEditorPlugin);

	/** Unload a plugin by its name. */
	void unloadPlugin(const QString &pluginName);

private slots:
	/** Load or unload a plugin when one is switching a checkbox in the options. */
	void loadOrUnload(QTableWidgetItem *item);
};

#endif // PLUGINMANAGER_H
