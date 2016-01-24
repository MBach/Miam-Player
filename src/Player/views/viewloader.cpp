#include "viewloader.h"

#include <uniquelibrary.h>
#include "viewplaylists.h"

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
	} // else {
		/// TODO other view loaded from plugins?
	//}
	return view;
}
