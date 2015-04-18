#include "mediaplayer.h"

#include "settings.h"
#include "settingsprivate.h"
#include "model/sqldatabase.h"
#include <QGuiApplication>
#include <QMediaContent>
#include <QMediaPlaylist>
#include <QWindow>

#include "remotemediaplayer.h"

#include "vlc-qt/Audio.h"
#include "vlc-qt/Common.h"
#include "vlc-qt/Instance.h"
#include "vlc-qt/Media.h"
#include "vlc-qt/MediaPlayer.h"

#include <QtDebug>

#include <QDir>

MediaPlayer* MediaPlayer::_mediaPlayer = NULL;

MediaPlayer::MediaPlayer(QObject *parent) :
	QObject(parent), _playlist(NULL), _state(QMediaPlayer::StoppedState), _media(NULL), _remotePlayer(NULL)
  , _stopAfterCurrent(false)
{
#ifdef Q_OS_OSX
	QDir d(QCoreApplication::applicationDirPath());
	d.cd("../PlugIns/");
	VlcCommon::setPluginPath(d.absolutePath());
#endif
	_instance = new VlcInstance(VlcCommon::args(), this);
	_player = new VlcMediaPlayer(_instance);
	this->createLocalConnections();

	connect(this, &MediaPlayer::currentMediaChanged, this, [=] (const QString &uri) {
		QWindow *w = QGuiApplication::topLevelWindows().first();
		TrackDAO t = SqlDatabase::instance()->selectTrack(uri);
		if (t.artist().isEmpty()) {
			w->setTitle(t.title() + " - Miam Player");
		} else {
			w->setTitle(t.title() + " (" + t.artist() + ") - Miam Player");
		}
	});

	// Link core multimedia actions
	connect(this, &MediaPlayer::mediaStatusChanged, this, [=] (QMediaPlayer::MediaStatus status) {
		if (status == QMediaPlayer::EndOfMedia) {
			if (_stopAfterCurrent) {
				stop();
				_stopAfterCurrent = false;
			} else {
				skipForward();
			}
		}
	});
}

MediaPlayer* MediaPlayer::instance()
{
	if (_mediaPlayer == NULL) {
		_mediaPlayer = new MediaPlayer;
	}
	return _mediaPlayer;
}

void MediaPlayer::createLocalConnections()
{
	// Disconnect remote players
	QMapIterator<QString, RemoteMediaPlayer*> it(_remotePlayers);
	while (it.hasNext()) {
		it.next();
		RemoteMediaPlayer *p = it.value();
		if (p) {
			p->disconnect();
		}
	}
	_remotePlayer = NULL;
	_player->disconnect();

	connect(_player, &VlcMediaPlayer::opening, this, [=]() {
		emit mediaStatusChanged(QMediaPlayer::LoadingMedia);
	});

	connect(_player, &VlcMediaPlayer::stopped, this, [=]() {
		//qDebug() << "VlcMediaPlayer::stopped";
		emit mediaStatusChanged(QMediaPlayer::NoMedia);
		this->setState(QMediaPlayer::StoppedState);
	});

	connect(_player, &VlcMediaPlayer::paused, this, [=]() {
		//qDebug() << "VlcMediaPlayer::paused";
		this->setState(QMediaPlayer::PausedState);
	});

	connect(_player, &VlcMediaPlayer::buffering, this, [=](float buffer) {
		//qDebug() << "VlcMediaPlayer::buffering" << buffer;
		if (buffer == 100) {
			_player->audio()->setVolume(Settings::instance()->volume());
			if (_state != QMediaPlayer::PlayingState && _state != QMediaPlayer::PausedState) {
				this->setState(QMediaPlayer::PlayingState);
			}
			// useless signal?
			//emit mediaStatusChanged(QMediaPlayer::BufferedMedia);
		} else {
			emit mediaStatusChanged(QMediaPlayer::BufferingMedia);
		}
	});

	connect(_player, &VlcMediaPlayer::end, this, [=]() {
		//qDebug() << "VlcMediaPlayer::end";
		emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
	});

	connect(_player, &VlcMediaPlayer::error, this, [=]() {
		//qDebug() << "VlcMediaPlayer::error";
		emit mediaStatusChanged(QMediaPlayer::InvalidMedia);
	});

	// VlcMediaPlayer::positionChanged is percent-based
	connect(_player, &VlcMediaPlayer::positionChanged, this, [=](float f) {
		//qDebug() << "VlcMediaPlayer::positionChanged";
		qint64 pos = _player->length() * f;
		emit positionChanged(pos, _player->length());
	});

	// Cannot use new signal/slot syntax because libvlc_media_t is not fully defined at compile time (just a forward declaration)
	connect(_player, SIGNAL(mediaChanged(libvlc_media_t*)), this, SLOT(convertMedia(libvlc_media_t*)));
}

void MediaPlayer::createRemoteConnections(const QUrl &track)
{
	// Disconnect all existing remote players
	QMapIterator<QString, RemoteMediaPlayer*> it(_remotePlayers);
	while (it.hasNext()) {
		it.next().value()->disconnect();
	}

	// Reconnect the good one
	RemoteMediaPlayer *p = _remotePlayers.value(track.host());
	if (!p) {
		_remotePlayer = NULL;
		return;
	}
	_remotePlayer = p;
	_remotePlayer->disconnect();
	_remotePlayer->setVolume(Settings::instance()->volume());

	// Disconnect local player first
	_player->disconnect();

	connect(_remotePlayer, &RemoteMediaPlayer::paused, this, [=]() {
		this->setState(QMediaPlayer::PausedState);
	});
	connect(_remotePlayer, &RemoteMediaPlayer::positionChanged, [=](qint64 pos, qint64 duration) {
		// Cannot set position with connect(...) in plugin. Maybe thread problem? (Tried all Qt::Connection in plugin)
		_remotePlayer->setPosition(pos);
		emit positionChanged(pos, duration);
	});
	connect(_remotePlayer, &RemoteMediaPlayer::started, this, [=](int duration) {
		_remotePlayer->setTime(duration);
		this->setState(QMediaPlayer::PlayingState);
		emit currentMediaChanged(track.toString());
	});
	connect(_remotePlayer, &RemoteMediaPlayer::stopped, this, [=]() {
		this->setState(QMediaPlayer::StoppedState);
	});
	connect(_remotePlayer, &RemoteMediaPlayer::trackHasEnded, this, [=]() {
		this->setState(QMediaPlayer::StoppedState);
		emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
	});
}

void MediaPlayer::addRemotePlayer(RemoteMediaPlayer *remotePlayer)
{
	if (remotePlayer) {
		_remotePlayers.insert(remotePlayer->host(), remotePlayer);
	}
}

void MediaPlayer::changeTrack(const QMediaContent &mediaContent)
{
	qDebug() << Q_FUNC_INFO << mediaContent.canonicalUrl();
	if (_remotePlayer) {
		_remotePlayer->disconnect();
		_remotePlayer->stop();
	} else {
		_player->disconnect();
		_player->stop();
	}
	_state = QMediaPlayer::StoppedState;
	this->playMediaContent(mediaContent);
}

void MediaPlayer::changeTrack(MediaPlaylist *playlist, int trackIndex)
{
	if (_remotePlayer) {
		_remotePlayer->disconnect();
		_remotePlayer->stop();
	} else {
		_player->disconnect();
		_player->stop();
	}
	_state = QMediaPlayer::StoppedState;
	_playlist = playlist;
	_playlist->setCurrentIndex(trackIndex);
	this->play();
}

void MediaPlayer::setVolume(int v)
{
	Settings::instance()->setVolume(v);
	if (_remotePlayer) {
		_remotePlayer->setVolume(v);
	} else {
		if (_player && _player->audio() && (_player->state() == Vlc::State::Playing || _player->state() == Vlc::State::Paused)) {
			_player->audio()->setVolume(v);
		} else {
			qDebug() << Q_FUNC_INFO << "couldn't set volume to" << v;
		}
	}
}

/** Current duration of the media, in ms. */
qint64 MediaPlayer::duration()
{
	if (_remotePlayer) {
		return _remotePlayer->length();
	} else {
		return _player->length();
	}
}

void MediaPlayer::playMediaContent(const QMediaContent &mc)
{
	// Everything is splitted in 2: local actions and remote actions
	if (mc.canonicalUrl().isLocalFile()) {
		this->createLocalConnections();

		// Resume playback is file was previously opened
		if (_state == QMediaPlayer::PausedState) {
			qDebug() << Q_FUNC_INFO << "QMediaPlayer::PausedState";
			_player->resume();
			this->setState(QMediaPlayer::PlayingState);
		} else {
			qDebug() << Q_FUNC_INFO << _state << mc.canonicalUrl().toLocalFile();
			QString file = mc.canonicalUrl().toLocalFile();
			if (_media) {
				delete _media;
			}
			_media = new VlcMedia(file, true, _instance);
			_player->open(_media);
		}
	} else {
		// Remote player is about to start
		this->createRemoteConnections(mc.canonicalUrl());
		if (_state == QMediaPlayer::PausedState) {
			_remotePlayer->resume();
		} else {
			_remotePlayer->play(mc.canonicalUrl());
		}
	}
}

/** Current position in the media, percent-based. */
float MediaPlayer::position() const
{
	if (_remotePlayer) {
		return _remotePlayer->position();
	} else {
		return _player->position();
	}
}

void MediaPlayer::setState(QMediaPlayer::State state)
{
	_state = state;
	emit stateChanged(_state);
}

/** Set mute on or off. */
void MediaPlayer::setMute(bool b) const
{
	if (_remotePlayer) {
		_remotePlayer->setMute(b);
	} else {
		// To avoid some glitches with VLC, it's better to switch from a fake track than actually muting sound
		if (b) {
			_player->audio()->setVolume(0);
		} else {
			_player->audio()->setVolume(Settings::instance()->volume());
		}
	}
}

void MediaPlayer::setTime(qint64 t) const
{
	if (_remotePlayer) {
		_remotePlayer->setTime(t);
	} else {
		_player->setTime(t);
	}
}

qint64 MediaPlayer::time() const
{
	if (_remotePlayer) {
		return _remotePlayer->time();
	} else {
		return _player->time();
	}
}



void MediaPlayer::seek(float pos)
{
	if (pos == 1.0) {
		pos -= 0.001f;
	}

	if (_remotePlayer) {
		_remotePlayer->seek(pos);
	} else {
		_player->setPosition(pos);
	}
}

/** Seek backward in the current playing track for a small amount of time. */
void MediaPlayer::seekBackward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		int currentPos = 0;
		if (_remotePlayer) {
			currentPos = _remotePlayer->position() * _remotePlayer->length();
		} else {
			currentPos = _player->time();
		}
		int time = currentPos - SettingsPrivate::instance()->playbackSeekTime();
		if (time < 0) {
			this->seek(0.0);
		} else {
			if (_remotePlayer) {
				this->seek(time / (float)_remotePlayer->length());
			} else {
				_player->setTime(time);
			}
		}
	}
}

/** Seek forward in the current playing track for a small amount of time. */
void MediaPlayer::seekForward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		if (_remotePlayer) {
			int currentPos = _remotePlayer->position() * _remotePlayer->length();
			int time = currentPos + SettingsPrivate::instance()->playbackSeekTime();
			if (time > _remotePlayer->length()) {
				skipForward();
			} else {
				this->seek(time / (float)_remotePlayer->length());
			}
		} else {
			int time = _player->time() + SettingsPrivate::instance()->playbackSeekTime();
			if (time > _player->length()) {
				skipForward();
			} else {
				_player->setTime(time);
			}
		}
	}
}

void MediaPlayer::skipBackward()
{
	if (!_playlist || (_playlist && _playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->previousIndex() < 0)) {
		return;
	}
	this->stop();
	_playlist->previous();
	this->play();
}

void MediaPlayer::skipForward()
{
	qDebug() << Q_FUNC_INFO;
	if (!_playlist || (_playlist && _playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->nextIndex() < _playlist->currentIndex())) {
		if (_state != QMediaPlayer::StoppedState) {
			this->stop();
		}
		return;
	}
	this->stop();
	_playlist->next();
	this->play();
}

/** Pause current playing track. */
void MediaPlayer::pause()
{
	if (!_playlist) {
		return;
	}
	if (_remotePlayer) {
		_remotePlayer->pause();
	} else {
		_player->pause();
	}
	this->setState(QMediaPlayer::PausedState);
}

/** Play current track in the playlist. */
void MediaPlayer::play()
{
	// Check if it's possible to play tracks first
	if (!_playlist) {
		return;
	}
	QMediaContent mc = _playlist->media(_playlist->currentIndex());
	if (mc.isNull()) {
		return;
	}

	this->playMediaContent(mc);
}

/** Stop current track in the playlist. */
void MediaPlayer::stop()
{
	if (_remotePlayer) {
		_remotePlayer->stop();
	} else {
		_player->stop();
	}
}

/** Activate or desactive audio output. */
void MediaPlayer::toggleMute() const
{
	if (_remotePlayer) {
		qDebug() << Q_FUNC_INFO << "not yet implemented for remote players";
	} else {
		if (_player->audio()->track() == 0) {
			_player->audio()->setTrack(-1);
		} else {
			_player->audio()->setTrack(0);
		}
	}
}

void MediaPlayer::convertMedia(libvlc_media_t *)
{
	emit currentMediaChanged("file://" + _player->currentMedia()->currentLocation());
}
