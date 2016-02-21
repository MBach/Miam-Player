#include "viewloader.h"

#include <uniquelibrary.h>
#include "viewplaylists.h"

ViewLoader::ViewLoader(MediaPlayer *mediaPlayer, PluginManager *pluginManager, QWidget *parent)
	: _mediaPlayer(mediaPlayer)
	, _pluginManager(pluginManager)
	, _parent(parent)
{

}

AbstractView* ViewLoader::load(AbstractView *currentView, const QString &menuAction)
{
	if (currentView) {
		QPair<QString, QObjectList> extensions = currentView->extensionPoints();
		_pluginManager->unregisterExtensionPoint(extensions.first);
	}

	AbstractView *view = nullptr;
	if (menuAction == "actionViewPlaylists" || menuAction == "actionViewUniqueLibrary") {

		if (menuAction == "actionViewPlaylists") {
			ViewPlaylists *viewPlaylists = new ViewPlaylists(_mediaPlayer, _parent);
			view = viewPlaylists;

		} else {
			UniqueLibrary *uniqueLibrary = new UniqueLibrary(_mediaPlayer, _parent);
			UniqueLibraryMediaPlayerControl *control = static_cast<UniqueLibraryMediaPlayerControl*>(uniqueLibrary->mediaPlayerControl());
			control->setUniqueLibrary(uniqueLibrary);
			view = uniqueLibrary;
		}
		this->attachPluginToBuiltInView(view);

	} else if (currentView != nullptr) {

		view = this->loadFromPlugin(currentView, menuAction);

	}
	return view;
}

/** Attach plugins if views allow to receive some. */
void ViewLoader::attachPluginToBuiltInView(AbstractView *view)
{
	QPair<QString, QObjectList> extensionPoints = view->extensionPoints();
	qDebug() << Q_FUNC_INFO << extensionPoints.first << extensionPoints.second;
	if (!extensionPoints.first.isEmpty()) {
		_pluginManager->registerExtensionPoint(extensionPoints);
	}

	for (BasicPlugin *plugin : _pluginManager->loadedPlugins().values()) {

		if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
			mediaPlayerPlugin->setMediaPlayerControl(view->mediaPlayerControl());
		} else if (RemoteMediaPlayerPlugin *remote = qobject_cast<RemoteMediaPlayerPlugin*>(plugin)) {

			/// XXX: This is ultra-specific!
			if (ViewPlaylists *vp = qobject_cast<ViewPlaylists*>(view)) {
				remote->setSearchDialog(vp->searchDialog());
			}
		}
	}
}

AbstractView* ViewLoader::loadFromPlugin(AbstractView *currentView, const QString &menuAction)
{
	AbstractView *view = nullptr;

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

		// Check if we need to pass some objects from old view to new view, because new one can be brought by plugins
		// For example, a plugin may need the MediaPlayerControl from the current view
		if (MediaPlayerPlugin *mediaPlayerPlugin = qobject_cast<MediaPlayerPlugin*>(plugin)) {
			qDebug() << Q_FUNC_INFO << mediaPlayerPlugin;
			mediaPlayerPlugin->setMediaPlayerControl(currentView->mediaPlayerControl());
			if (mediaPlayerPlugin->hasView()) {
				view = mediaPlayerPlugin->instanciateView();
				view->setOrigin(currentView);
			}
		}
	}
	return view;
}
