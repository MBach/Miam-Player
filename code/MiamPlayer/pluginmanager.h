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
	/** Reference to the MainWindow instance (strong coupling). */
	MainWindow *_mainWindow;

	/** Loaded plugins are stored in this map. */
	QMap<QString, BasicPlugin*> _loadedPlugins;

	/** Every plugin might instanciate objects that we need to be able to delete later (especially for unloading). */
	QMultiMap<QString, QObject*> _dependencies;

	/** Plugins are stored in a subdirectory called "plugins" under the application path. */
	QString _pluginPath;

	/** Some instances in the software can be modified (menus, buttons, widgets, etc). */
	//QMultiMap<QString, QObject*> _extensionPoints;

public:
	explicit PluginManager(MainWindow *parent);

	/** Explicitly destroys every plugin. */
	virtual ~PluginManager();

	/** Display a QMessageBox if at least one error was encountered when loading plugins. */
	void alertUser(const QStringList &failedPlugins);

	inline QMap<QString, BasicPlugin*> loadedPlugins() const { return _loadedPlugins; }

	/** Load a plugin by its location on the hard drive. */
	bool loadPlugin(const QString &fileName);

	/** Allow views to be extended by adding 1 or more entries in a context menu and items to interact with. */
	//void registerExtensionPoint(const char *className, QObjectList target);

	/** Unload a plugin by its name. */
	bool unloadPlugin(const QString &fileName);

private:
	void loadItemViewPlugin(ItemViewPlugin *itemViewPlugin);
	void loadMediaPlayerPlugin(MediaPlayerPlugin *mediaPlayerPlugin);
	void loadRemoteMediaPlayerPlugin(RemoteMediaPlayerPlugin *remoteMediaPlayerPlugin);
	void loadSearchMediaPlayerPlugin(SearchMediaPlayerPlugin *searchMediaPlayerPlugin);
	void loadTagEditorPlugin(TagEditorPlugin *tagEditorPlugin);
};

#endif // PLUGINMANAGER_H
