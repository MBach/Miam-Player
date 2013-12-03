#include "mediaplayer.h"

#include "settings.h"

#include <QMediaPlaylist>

MediaPlayer::MediaPlayer(QObject *parent) :
	QMediaPlayer(parent, QMediaPlayer::StreamPlayback)
{
	this->setNotifyInterval(100);
}

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
	if (playlist()) {
		playlist()->previous();
		play();
	}
}

void MediaPlayer::skipForward()
{
	if (playlist()) {
		playlist()->next();
		play();
	}
}
