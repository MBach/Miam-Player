#include "viewloader.h"

#include "viewplaylists.h"

ViewLoader::ViewLoader(MediaPlayer *mediaPlayer)
	: _mediaPlayer(mediaPlayer)
{

}

AbstractView* ViewLoader::load(const QString &action)
{
	AbstractView *view = nullptr;
	if (action == "actionViewPlaylists") {
		ViewPlaylists *viewPlaylists = new ViewPlaylists(_mediaPlayer);
		view = viewPlaylists;
	}
	return view;
}
