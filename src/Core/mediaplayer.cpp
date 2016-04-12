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
	connect(_localPlayer, &QtAV::AVPlayer::stopped, this, [=]() {
		//qDebug() << "QtAV::AVPlayer::stopped";
		this->setState(QMediaPlayer::StoppedState);
	});

	connect(_localPlayer, &QtAV::AVPlayer::loaded, this, [=]() {
		//qDebug() << "QtAV::AVPlayer::loaded";
		_localPlayer->audio()->setVolume(Settings::instance()->volume());
		emit currentMediaChanged(_localPlayer->file());
		this->setState(QMediaPlayer::PlayingState);
	});

	connect(_localPlayer, &QtAV::AVPlayer::paused, this, [=](bool) {
		//qDebug() << "QtAV::AVPlayer::paused" << b;
		this->setState(QMediaPlayer::PausedState);
	});

	connect(_localPlayer, &QtAV::AVPlayer::positionChanged, this, [=](qint64 pos) {
		emit positionChanged(pos, _localPlayer->duration());
	});

	connect(this, &MediaPlayer::currentMediaChanged, this, [=] (const QString &uri) {
		QWindow *w = QGuiApplication::topLevelWindows().first();
		TrackDAO t = SqlDatabase().selectTrackByURI(uri);
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

void MediaPlayer::addRemotePlayer(IMediaPlayer *remotePlayer)
{
	if (remotePlayer) {
		_remotePlayers.insert(remotePlayer->host(), remotePlayer);
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

void MediaPlayer::setVolume(qreal v)
{
	Settings::instance()->setVolume(v);
	if (_remotePlayer) {
		_remotePlayer->setVolume(v);
	} else {
		_localPlayer->audio()->setVolume(v);
	}
}

void MediaPlayer::playMediaContent(const QMediaContent &mc)
{
	if ((_state == QMediaPlayer::PlayingState) || (_state == QMediaPlayer::PausedState)) {
		this->stop();
	}

	// Everything is splitted in 2: local actions and remote actions
	if (mc.canonicalUrl().isLocalFile()) {
		_localPlayer->play(mc.canonicalUrl().toLocalFile());
	} else {
		// Find remote player attached to mediaContent
		_remotePlayer = _remotePlayers.value(mc.canonicalUrl().host());
		qDebug() << Q_FUNC_INFO << "about to play remote track" << mc.canonicalUrl().host() << _remotePlayer;
		if (_remotePlayer) {
			qDebug() << Q_FUNC_INFO << "about to play remote track" << mc.canonicalUrl();
			_remotePlayer->play(mc.canonicalUrl());
		} else {
			qDebug() << Q_FUNC_INFO << "couldn't get remote player for" << mc.canonicalUrl();
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
	switch (state) {
	case QMediaPlayer::StoppedState:
		emit mediaStatusChanged(QMediaPlayer::EndOfMedia);
		break;
	case QMediaPlayer::PlayingState:
		break;
	case QMediaPlayer::PausedState:
		break;
	}
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
			this->seek(time / (qreal)duration);
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
	qDebug() << Q_FUNC_INFO << playlist();
	if (!_playlist) {
		return;
	}
	if (_playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->previousIndex() < 0) {
		return;
	}
	_state = QMediaPlayer::StoppedState;
	_playlist->skipBackward();
	this->play();
}

void MediaPlayer::skipForward()
{
	if (!_playlist) {
		return;
	}
	if (_playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->nextIndex() < _playlist->currentIndex()) {
		return;
	}
	_state = QMediaPlayer::StoppedState;
	_playlist->skipForward();
	this->play();
}

/** Pause current playing track. */
void MediaPlayer::pause()
{
	if (_remotePlayer) {
		_remotePlayer->pause();
	} else {
		_localPlayer->pause(true);
	}
	_state = QMediaPlayer::PausedState;
}

/** Play current track in the playlist. */
void MediaPlayer::play()
{
	// Check if it's possible to play tracks first
	if (!_playlist) {
		qDebug() << Q_FUNC_INFO << "no playlist!";
		return;
	}
	QMediaContent mc = _playlist->media(_playlist->currentIndex());
	if (mc.isNull()) {
		qDebug() << Q_FUNC_INFO << "media content is null!";
		return;
	}
	this->playMediaContent(mc);
}

/** Stop current track in the playlist. */
void MediaPlayer::stop()
{
	if (_state != QMediaPlayer::StoppedState) {
		if (_remotePlayer) {
			_remotePlayer->stop();
		} else {
			_localPlayer->stop();
		}
		_state = QMediaPlayer::StoppedState;
	}
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

/** Play or pause current track in the playlist depending of the state of the player. */
void MediaPlayer::togglePlayback()
{
	switch (_state) {
	case QMediaPlayer::PausedState:
		this->resume();
		break;
	case QMediaPlayer::StoppedState:
		this->play();
		break;
	case QMediaPlayer::PlayingState:
		this->pause();
		break;
	}
}

void MediaPlayer::resume()
{
	if (_remotePlayer) {
		_remotePlayer->resume();
	} else {
		_localPlayer->pause(false);
	}
	this->setState(QMediaPlayer::PlayingState);
}
