#include "viewplaylists.h"

ViewPlaylists::ViewPlaylists(MediaPlayer *mediaPlayer)
	: _mediaPlayer(mediaPlayer)
{
	this->setupUi(this);
	seekSlider->setMediaPlayer(_mediaPlayer);
	tabPlaylists->init(_mediaPlayer);
}
