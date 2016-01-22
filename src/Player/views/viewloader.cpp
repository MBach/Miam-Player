#include "viewloader.h"

#include "viewplaylists.h"
#include <uniquelibrary.h>

ViewLoader::ViewLoader(MediaPlayer *mediaPlayer)
	: _mediaPlayer(mediaPlayer)
{

}

AbstractView* ViewLoader::load(const QString &menuAction)
{
	AbstractView *view = nullptr;
	if (menuAction == "actionViewPlaylists") {
		ViewPlaylists *viewPlaylists = new ViewPlaylists(_mediaPlayer);
		view = viewPlaylists;
	} else if (menuAction == "actionViewUniqueLibrary") {
		UniqueLibrary *uniqueLibrary = new UniqueLibrary(_mediaPlayer, nullptr);
		view = uniqueLibrary;
	} else if (menuAction == "actionViewTagEditor") {
	} else {
		/// TODO other view loaded from plugins?
	}
	return view;
}
