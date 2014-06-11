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
	/// FIXME ?
	//this->setNotifyInterval(100);
	_instance = new VlcInstance(VlcCommon::args(), this);
	_player = new VlcMediaPlayer(_instance);

	connect(_player, &VlcMediaPlayer::opening, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::LoadingMedia);
	});

	connect(_player, &VlcMediaPlayer::playing, this, [=]() {
		// Prevent multiple signals bug?
		if (_state != QMediaPlayer::PlayingState) {
			emit mediaStatusChanged(QMediaPlayer::LoadedMedia);
			_state = QMediaPlayer::PlayingState;
			emit stateChanged(QMediaPlayer::PlayingState);
		}
	});

	connect(_player, &VlcMediaPlayer::stopped, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::NoMedia);
		_state = QMediaPlayer::StoppedState;
		emit stateChanged(QMediaPlayer::StoppedState);
	});

	connect(_player, &VlcMediaPlayer::paused, this, [=]() {
		_state = QMediaPlayer::PausedState;
		emit stateChanged(QMediaPlayer::PausedState);
	});

	connect(_player, &VlcMediaPlayer::buffering, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::BufferingMedia);
	});

	connect(_player, &VlcMediaPlayer::end, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
	});

	connect(_player, &VlcMediaPlayer::error, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::InvalidMedia);
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
		qDebug() << "currentIndexChanged";
		_state = QMediaPlayer::StoppedState;
	});
	connect(_playlist, &QMediaPlaylist::mediaRemoved, this, [=](int start, int end) {
		qDebug() << "mediaRemoved" << start << end << _playlist->currentIndex();
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
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		int currentPos = _player->position() * _player->length();
		int time = currentPos - Settings::getInstance()->playbackSeekTime();
		if (time < 0) {
			_player->setPosition(0.0);
		} else {
			_player->setPosition(time / (float)_player->length());
		}
	}
}

/** Seek forward in the current playing track for a small amount of time. */
void MediaPlayer::seekForward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		int currentPos = _player->position() * _player->length();
		int time = currentPos + Settings::getInstance()->playbackSeekTime();
		if (time > _player->length()) {
			skipForward();
		} else {
			_player->setPosition(time / (float)_player->length());
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
