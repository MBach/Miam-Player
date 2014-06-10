#include "mediaplayer.h"

#include "settings.h"
#include <QMediaPlaylist>

#include "vlc-qt/Audio.h"
#include "vlc-qt/Common.h"
#include "vlc-qt/Instance.h"
#include "vlc-qt/Media.h"
#include "vlc-qt/MediaPlayer.h"

#include <QtDebug>

MediaPlayer::MediaPlayer(QObject *parent) :
	QObject(parent), _media(NULL), _playlist(NULL)
{
	/// FIXME
	//this->setNotifyInterval(100);
	_instance = new VlcInstance(VlcCommon::args(), this);
	_player = new VlcMediaPlayer(_instance);

	connect(_player, &VlcMediaPlayer::stateChanged, this, [=]() {
		qDebug() << "VlcMediaPlayer::stateChanged";
		switch (_player->state()) {
		case Vlc::Idle:
			qDebug() << "Idle";
			break;
		case Vlc::Opening:
			emit mediaStatusChanged(QMediaPlayer::LoadingMedia);
			qDebug() << "Opening";
			break;
		case Vlc::Buffering:
			emit mediaStatusChanged(QMediaPlayer::BufferingMedia);
			qDebug() << "Buffering";
			break;
		case Vlc::Playing: {
			qDebug() << "Playing";
			/// XXX: prevent multiple signals bug?
			emit mediaStatusChanged(QMediaPlayer::LoadedMedia);
			_state = QMediaPlayer::PlayingState;
			emit stateChanged(QMediaPlayer::PlayingState);
			break;
		}
		case Vlc::Paused:
			qDebug() << "Paused";
			_state = QMediaPlayer::PausedState;
			emit stateChanged(QMediaPlayer::PausedState);
			break;
		case Vlc::Stopped:
			qDebug() << "Stopped";
			emit mediaStatusChanged(QMediaPlayer::NoMedia);
			_state = QMediaPlayer::StoppedState;
			emit stateChanged(QMediaPlayer::StoppedState);
			break;
		case Vlc::Ended:
			qDebug() << "Ended";
			emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
			break;
		case Vlc::Error:
			qDebug() << "Error";
			emit mediaStatusChanged(QMediaPlayer::InvalidMedia);
			break;
		}
	});

	connect(_player, &VlcMediaPlayer::positionChanged, this, [=](float f) {
		qint64 pos = _player->length() * f;
		emit positionChanged(pos);
	});

	// Cannot use new signal/slot syntax because libvlc_media_t is not fully defined at compile time (just a forward declaration)
	/*connect(_player, &VlcMediaPlayer::mediaChanged, this, [=](){
		//qDebug() << f;
		QMediaContent mc(_player->currentMedia()->currentLocation());
		emit currentMediaChanged(mc);
	});*/
	connect(_player, SIGNAL(mediaChanged(libvlc_media_t*)), this, SLOT(convertMedia(libvlc_media_t*)));


}

QMediaPlaylist * MediaPlayer::playlist()
{
	return _playlist;
}

void MediaPlayer::setPlaylist(QMediaPlaylist *playlist)
{
	_playlist = playlist;
	connect(_playlist, &QMediaPlaylist::currentIndexChanged, this, [=]() {
		_state = QMediaPlayer::StoppedState;
	});
	connect(_playlist, &QMediaPlaylist::mediaRemoved, this, [=](int start, int end) {
		qDebug() << "mediaRemoved" << start << end;
		if (_playlist->isEmpty() || _playlist->currentIndex() == start) {
			_player->stop();
		}
	});
}

void MediaPlayer::setVolume(int v)
{
	_player->audio()->setVolume(v);
}

qint64 MediaPlayer::duration()
{
	return _player->length();
}

QMediaPlayer::State MediaPlayer::state() const
{
	return _state;
}

/** Seek backward in the current playing track for a small amount of time. */
void MediaPlayer::seekBackward()
{
	/*if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		qint64 time = (qint64)_player->length() - Settings::getInstance()->playbackSeekTime();
		int p = _player->length() * _player->position();
		qDebug() << "length" << _player->length() << "pos" << _player->position() << "p" << p;
		float pos = (p - time) / (float)_player->length();
		qDebug() << "new pos" << pos;
		if (pos < 0) {
			//_player->setTime(1);
			_player->setPosition(0.0);
		} else {

			_player->setPosition(pos);
		}
	}*/
}

/** Seek forward in the current playing track for a small amount of time. */
void MediaPlayer::seekForward()
{
	/*if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		qint64 time = position() + Settings::getInstance()->playbackSeekTime();
		if (time > duration()) {
			skipForward();
		} else {
			setPosition(time);
		}
	}*/
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
		if (playlist()->playbackMode() == QMediaPlaylist::Sequential && playlist()->nextIndex() < playlist()->currentIndex()) {
			return;
		}
		playlist()->next();
		play();
	}
}

void MediaPlayer::pause()
{
	_player->pause();
}

void MediaPlayer::play()
{
	if (playlist()) {
		if (_state == QMediaPlayer::PausedState) {
			_player->resume();
		} else {
			QMediaContent mc = playlist()->media(playlist()->currentIndex());
			if (!mc.isNull()) {
				QString file = mc.canonicalUrl().toLocalFile();
				if (_media) {
					_media->disconnect();
					delete _media;
				}
				_media = new VlcMedia(file, true, _instance);
				/*connect(_media, &VlcMedia::durationChanged, this, [=](int i){
					qDebug() << "durationChanged" << i;
				});*/
				qDebug() << "MediaPlayer::play()";
				_player->open(_media);
			}
		}
	}
}

void MediaPlayer::stop()
{
	_player->stop();
}

void MediaPlayer::convertMedia(libvlc_media_t *)
{
	QUrl url = QUrl::fromLocalFile(_player->currentMedia()->currentLocation());
	QMediaContent mc(url);
	emit currentMediaChanged(mc);
}
