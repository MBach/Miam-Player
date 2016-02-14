#include "viewloader.h"

#include <uniquelibrary.h>
#include "viewplaylists.h"

ViewLoader::ViewLoader(MediaPlayer *mediaPlayer, PluginManager *pluginManager, QWidget *parent)
	: _mediaPlayer(mediaPlayer)
	, _pluginManager(pluginManager)
	, _parent(parent)
{

}

AbstractView* ViewLoader::load(const QString &menuAction)
{
	AbstractView *view = nullptr;
	if (menuAction == "actionViewPlaylists") {
		ViewPlaylists *viewPlaylists = new ViewPlaylists(_mediaPlayer, _parent);
		view = viewPlaylists;
	} else if (menuAction == "actionViewUniqueLibrary") {
		UniqueLibrary *uniqueLibrary = new UniqueLibrary(_mediaPlayer, _parent);
		UniqueLibraryMediaPlayerControl *control = static_cast<UniqueLibraryMediaPlayerControl*>(uniqueLibrary->mediaPlayerControl());
		control->setUniqueLibrary(uniqueLibrary);
		view = uniqueLibrary;
	}  /*else {
		// Other views loaded from plugins
		QMultiMap<QString, QObject*> multiMap = _pluginManager->dependencies();
		QObjectList dep = multiMap.values(menuAction);
		if (dep.isEmpty()) {
			return view;
		}
		qDebug() << Q_FUNC_INFO << "No built-in view was found for this action. Was it from an external plugin?";
		for (BasicPlugin *plugin : _pluginManager->loadedPlugins().values()) {
			if (plugin->name() != menuAction) {
				continue;
			}
			qDebug() << Q_FUNC_INFO << plugin;

			if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
				return mediaPlayerPlugin->instanciateView();
			}
		}
	}*/
	if (view) {
		for (BasicPlugin *plugin : _pluginManager->loadedPlugins().values()) {
			if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
				mediaPlayerPlugin->setMediaPlayerControl(view->mediaPlayerControl());
				if (mediaPlayerPlugin->hasView()) {
					return mediaPlayerPlugin->instanciateView();
				}

			}
		}
	}
	return view;
}
