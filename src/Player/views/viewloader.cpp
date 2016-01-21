#include "viewloader.h"

#include "viewplaylists.h"

ViewLoader::ViewLoader(MediaPlayer *mediaPlayer)
	: _mediaPlayer(mediaPlayer)
{

}

AbstractView* ViewLoader::load(QMenu *menuPlaylist, const QString &menuAction)
{
	AbstractView *view = nullptr;
	if (menuAction == "actionViewPlaylists") {
		ViewPlaylists *viewPlaylists = new ViewPlaylists(menuPlaylist, _mediaPlayer);
		view = viewPlaylists;
	} else if (menuAction == "actionViewUniqueLibrary") {
	} else if (menuAction == "actionViewTagEditor") {
	} else {
		/// TODO other view loaded from plugins?
	}
	return view;
}
