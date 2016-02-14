#include "viewplaylistsmediaplayercontrol.h"

ViewPlaylistsMediaPlayerControl::ViewPlaylistsMediaPlayerControl(MediaPlayer *mediaPlayer, QObject *parent)
	: MediaPlayerControl(mediaPlayer, parent)
{

}

/** Returns true if current playlist has a plabackMode to Random. */
bool ViewPlaylistsMediaPlayerControl::isInShuffleState() const
{
	if (!mediaPlayer()->playlist()) {
		return false;
	}
	return mediaPlayer()->playlist()->playbackMode() == QMediaPlaylist::Random;
}

/** Forward action to MediaPlayer. */
void ViewPlaylistsMediaPlayerControl::skipBackward()
{
	mediaPlayer()->skipBackward();
}

/** Forward action to MediaPlayer. */
void ViewPlaylistsMediaPlayerControl::skipForward()
{
	mediaPlayer()->skipForward();
}

/** Forward action to MediaPlayer. */
void ViewPlaylistsMediaPlayerControl::stop()
{
	mediaPlayer()->stop();
}

/** Forward action to MediaPlayer. */
void ViewPlaylistsMediaPlayerControl::togglePlayback()
{
	mediaPlayer()->togglePlayback();
}

void ViewPlaylistsMediaPlayerControl::toggleShuffle(bool checked)
{
	if (checked) {
		mediaPlayer()->playlist()->setPlaybackMode(QMediaPlaylist::Random);
	} else {
		mediaPlayer()->playlist()->setPlaybackMode(QMediaPlaylist::Sequential);
	}
}
