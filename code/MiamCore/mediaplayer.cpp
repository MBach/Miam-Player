#include "mediaplayer.h"

#include "settings.h"

#include <QMediaPlaylist>

MediaPlayer::MediaPlayer(QObject *parent) :
	QMediaPlayer(parent, QMediaPlayer::StreamPlayback)
{
	this->setNotifyInterval(100);
}

/*void MediaPlayer::play(const QModelIndex &index)
{
	if (state() == QMediaPlayer::PlayingState) {
		blockSignals(true);
		stop();
		setPlaylist(currentPlayList()->mediaPlaylist());
		currentPlayList()->mediaPlaylist()->setCurrentIndex(index.row());
		play();
		blockSignals(false);
	} else {
		setPlaylist(currentPlayList()->mediaPlaylist());
		currentPlayList()->mediaPlaylist()->setCurrentIndex(index.row());
		play();
	}
}*/

/** Seek backward in the current playing track for a small amount of time. */
void MediaPlayer::seekBackward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		qint64 time = position() - Settings::getInstance()->playbackSeekTime();
		if (time < 0) {
			setPosition(1);
		} else {
			setPosition(time);
		}
	}
}

/** Seek forward in the current playing track for a small amount of time. */
void MediaPlayer::seekForward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		qint64 time = position() + Settings::getInstance()->playbackSeekTime();
		if (time > duration()) {
			skipForward();
		} else {
			setPosition(time);
		}
	}
}

void MediaPlayer::skipBackward()
{
	//setPlaylist(currentPlayList()->mediaPlaylist());
	if (state() == QMediaPlayer::PlayingState) {
		blockSignals(true);
		stop();
		playlist()->previous();
		play();
		blockSignals(false);
	} else {
		//currentPlayList()->mediaPlaylist()->previous();
		playlist()->previous();
	}
}

void MediaPlayer::skipForward()
{
	//setPlaylist(currentPlayList()->mediaPlaylist());
	if (state() == QMediaPlayer::PlayingState) {
		blockSignals(true);
		stop();
		playlist()->next();
		play();
		blockSignals(false);
	} else {
		//currentPlayList()->mediaPlaylist()->next();
		playlist()->next();
	}
}
