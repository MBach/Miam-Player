#include "mediaplayer.h"

#include "settings.h"
#include "settingsprivate.h"
#include "model/sqldatabase.h"
#include <QDir>
#include <QGuiApplication>
#include <QMediaContent>
#include <QMediaPlaylist>
#include <QWindow>

#include "imediaplayer.h"

#include <QtDebug>

#include <QtAV/AVPlayer.h>

MediaPlayer::MediaPlayer(QObject *parent)
	: QObject(parent)
	, _playlist(nullptr)
	, _state(QMediaPlayer::StoppedState)
	, _localPlayer(new QtAV::AVPlayer(this))
	, _remotePlayer(nullptr)
	, _stopAfterCurrent(false)
{
	this->createLocalConnections();

	connect(this, &MediaPlayer::currentMediaChanged, this, [=] (const QString &uri) {
		QWindow *w = QGuiApplication::topLevelWindows().first();
		TrackDAO t = SqlDatabase::instance()->selectTrackByURI(uri);
		if (t.artist().isEmpty()) {
			w->setTitle(t.title() + " - Miam Player");
		} else {
			w->setTitle(t.title() + " (" + t.artist() + ") - Miam Player");
		}
	});

	// Link core multimedia actions
	connect(this, &MediaPlayer::mediaStatusChanged, this, [=] (QMediaPlayer::MediaStatus status) {
		if (_state != QMediaPlayer::StoppedState && status == QMediaPlayer::EndOfMedia) {
			if (_stopAfterCurrent) {
				stop();
				_stopAfterCurrent = false;
			} else {
				skipForward();
			}
		}
	});
}

void MediaPlayer::createLocalConnections()
{
	// Disconnect remote players
	QMapIterator<QString, IMediaPlayer*> it(_remotePlayers);
	while (it.hasNext()) {
		it.next();
		IMediaPlayer *p = it.value();
		if (p) {
			p->disconnect();
		}
	}
	_remotePlayer = nullptr;
	_localPlayer->disconnect();

	connect(_localPlayer, &QtAV::AVPlayer::mediaStatusChanged, this, [=](QtAV::MediaStatus status) {
		switch (status) {
		case QtAV::LoadingMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "LoadingMedia";
			emit mediaStatusChanged(QMediaPlayer::LoadingMedia);
			break;
		case QtAV::NoMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "NoMedia";
			emit mediaStatusChanged(QMediaPlayer::NoMedia);
			this->setState(QMediaPlayer::StoppedState);
			break;
		case QtAV::BufferingMedia:
			//qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "BufferingMedia";
			emit mediaStatusChanged(QMediaPlayer::BufferingMedia);
			break;
		case QtAV::BufferedMedia:
			//qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "BufferedMedia";
			_localPlayer->audio()->setVolume(Settings::instance()->volume());
			/*if (_state != QMediaPlayer::PlayingState && _state != QMediaPlayer::PausedState) {
				this->setState(QMediaPlayer::PlayingState);
			}*/
			break;
		case QtAV::EndOfMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "EndOfMedia";
			emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
			break;
		case QtAV::InvalidMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "InvalidMedia";
			emit mediaStatusChanged(QMediaPlayer::InvalidMedia);
			break;
		case QtAV::UnknownMediaStatus:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "UnknownMediaStatus";
			break;
		case QtAV::LoadedMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "LoadedMedia";
			//if (_state != QMediaPlayer::PlayingState && _state != QMediaPlayer::PausedState) {
			emit currentMediaChanged("file://" + _localPlayer->file());
			this->setState(QMediaPlayer::PlayingState);
			//}
			break;
		case QtAV::StalledMedia:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << "StalledMedia";
			break;
		default:
			qDebug() << "QtAV::AVPlayer::mediaStatusChanged" << status;
			break;
		}
	});

	connect(_localPlayer, &QtAV::AVPlayer::paused, this, [=](bool) {
		qDebug() << "paused";
		this->setState(QMediaPlayer::PausedState);
	});

	connect(_localPlayer, &QtAV::AVPlayer::positionChanged, this, [=](qint64 pos) {
		emit positionChanged(pos, _localPlayer->duration());
	});
}

void MediaPlayer::createRemoteConnections(const QUrl &track)
{
	// Disconnect all existing remote players
	QMapIterator<QString, IMediaPlayer*> it(_remotePlayers);
	while (it.hasNext()) {
		it.next().value()->disconnect();
	}

	// Reconnect the good one
	IMediaPlayer *p = _remotePlayers.value(track.host());
	if (!p) {
		_remotePlayer = nullptr;
		return;
	}
	_remotePlayer = p;
	_remotePlayer->disconnect();
	_remotePlayer->setVolume(Settings::instance()->volume());

	// Disconnect local player first
	_localPlayer->disconnect();

	connect(_remotePlayer, &IMediaPlayer::paused, this, [=]() {
		this->setState(QMediaPlayer::PausedState);
	});
	connect(_remotePlayer, &IMediaPlayer::positionChanged, [=](qint64 pos, qint64 duration) {
		// Cannot set position with connect(...) in plugin. Maybe thread problem? (Tried all Qt::Connection in plugin)
		_remotePlayer->setPosition(pos);
		emit positionChanged(pos, duration);
	});
	connect(_remotePlayer, &IMediaPlayer::started, this, [=](int duration) {
		_remotePlayer->setTime(duration);
		this->setState(QMediaPlayer::PlayingState);
		emit currentMediaChanged(track.toString());
	});
	connect(_remotePlayer, &IMediaPlayer::stopped, this, [=]() {
		this->setState(QMediaPlayer::StoppedState);
	});
	connect(_remotePlayer, &IMediaPlayer::trackHasEnded, this, [=]() {
		this->setState(QMediaPlayer::StoppedState);
		emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
	});
}

void MediaPlayer::addRemotePlayer(IMediaPlayer *remotePlayer)
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
		_localPlayer->disconnect();
		_localPlayer->stop();
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
		_localPlayer->disconnect();
		_localPlayer->stop();
	}
	_state = QMediaPlayer::StoppedState;
	_playlist = playlist;
	if (_playlist->playbackMode() == QMediaPlaylist::Random) {
		_playlist->shuffle(trackIndex);
	} else {
		_playlist->setCurrentIndex(trackIndex);
	}
	this->play();
}

void MediaPlayer::setVolume(qreal v)
{
	Settings::instance()->setVolume(v);
	if (_remotePlayer) {
		_remotePlayer->setVolume(v);
	} else {
		_localPlayer->audio()->setVolume(v);
	}
}

/** Current duration of the media, in ms. */
qint64 MediaPlayer::duration()
{
	if (_remotePlayer) {
		return _remotePlayer->duration();
	} else {
		return _localPlayer->duration();
	}
}

void MediaPlayer::playMediaContent(const QMediaContent &mc)
{
	// Everything is splitted in 2: local actions and remote actions
	if (mc.canonicalUrl().isLocalFile()) {
		this->createLocalConnections();

		// Resume playback is file was previously opened
		if (_state == QMediaPlayer::PausedState) {
			_localPlayer->togglePause();
			this->setState(QMediaPlayer::PlayingState);
		} else {
			_localPlayer->play(mc.canonicalUrl().toLocalFile());
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
	this->setVolume(Settings::instance()->volume());
}

/** Current position in the media, percent-based. */
float MediaPlayer::position() const
{
	if (_remotePlayer) {
		return _remotePlayer->position();
	} else {
		return _localPlayer->position();
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
		_localPlayer->audio()->setMute(b);
	}
}

/*void MediaPlayer::setTime(qint64 t) const
{
	if (_remotePlayer) {
		_remotePlayer->setTime(t);
	} else {
		//_localPlayer->setTime(t);
	}
}*/

/*qint64 MediaPlayer::time() const
{
	if (_remotePlayer) {
		return _remotePlayer->time();
	} else {
		//return _localPlayer->time();
		return 0;
	}
}*/

void MediaPlayer::seek(qreal pos)
{
	if (pos == 1.0) {
		pos -= 0.001f;
	}

	if (_remotePlayer) {
		_remotePlayer->seek(pos);
	} else {
		_localPlayer->seek(pos);
	}
}

/** Seek backward in the current playing track for a small amount of time. */
void MediaPlayer::seekBackward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		qint64 currentPos = 0;
		qint64 duration = 0;
		if (_remotePlayer) {
			currentPos = _remotePlayer->position() * _remotePlayer->duration();
			duration = _remotePlayer->duration();
		} else {
			currentPos = _localPlayer->position();
			duration = _localPlayer->duration();
		}
		qint64 time = currentPos - SettingsPrivate::instance()->playbackSeekTime();
		if (time < 0 || duration == 0) {
			this->seek(0.0);
		} else {
			this->seek(time / duration);
		}
	}
}

/** Seek forward in the current playing track for a small amount of time. */
void MediaPlayer::seekForward()
{
	if (state() == QMediaPlayer::PlayingState || state() == QMediaPlayer::PausedState) {
		if (_remotePlayer) {
			qint64 currentPos = _remotePlayer->position() * _remotePlayer->duration();
			qint64 time = currentPos + SettingsPrivate::instance()->playbackSeekTime();
			if (time > _remotePlayer->duration()) {
				skipForward();
			} else {
				this->seek(time / (qreal)_remotePlayer->duration());
			}
		} else {
			qint64 time = _localPlayer->position() + SettingsPrivate::instance()->playbackSeekTime();
			if (time > _localPlayer->duration()) {
				skipForward();
			} else {
				this->seek(time / (qreal)_localPlayer->duration());
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
	_playlist->skipBackward();
	this->play();
}

void MediaPlayer::skipForward()
{
	if (!_playlist || (_playlist && _playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->nextIndex() < _playlist->currentIndex())) {
		if (_state != QMediaPlayer::StoppedState) {
			this->stop();
		}
		return;
	}
	this->stop();
	_playlist->skipForward();
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
		_localPlayer->pause();
	}
	qDebug() << Q_FUNC_INFO;
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
		_localPlayer->stop();
	}
	this->setState(QMediaPlayer::StoppedState);
}

/** Activate or desactive audio output. */
void MediaPlayer::toggleMute() const
{
	if (_remotePlayer) {
		qDebug() << Q_FUNC_INFO << "not yet implemented for remote players";
	} else {
		_localPlayer->audio()->setMute(!_localPlayer->audio()->isMute());
	}
}
