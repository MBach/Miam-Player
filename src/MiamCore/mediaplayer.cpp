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
#include <qxt/qxtglobalshortcut.h>

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

	// Init default multimedia keys
	QxtGlobalShortcut *shortcutSkipBackward = new QxtGlobalShortcut(QKeySequence(Qt::Key_MediaPrevious), this);
	QxtGlobalShortcut *shortcutStop = new QxtGlobalShortcut(Qt::Key_MediaStop, this);
	QxtGlobalShortcut *shortcutPlayPause = new QxtGlobalShortcut(Qt::Key_MediaPlay, this);
	QxtGlobalShortcut *shortcutSkipForward = new QxtGlobalShortcut(Qt::Key_MediaNext, this);

	connect(shortcutSkipBackward, &QxtGlobalShortcut::activated, this, &MediaPlayer::skipBackward);
	connect(shortcutPlayPause, &QxtGlobalShortcut::activated, this, &MediaPlayer::togglePlayback);
	connect(shortcutStop, &QxtGlobalShortcut::activated, this, &MediaPlayer::stop);
	connect(shortcutSkipForward, &QxtGlobalShortcut::activated, this, &MediaPlayer::skipForward);
}

void MediaPlayer::addRemotePlayer(IMediaPlayer *remotePlayer)
{
	if (remotePlayer) {
		_remotePlayers.insert(remotePlayer->host(), remotePlayer);
	}
}

/*void MediaPlayer::changeTrack(MediaPlaylist *playlist, int trackIndex)
{
	_state = QMediaPlayer::StoppedState;
	_playlist = playlist;
	if (_playlist->playbackMode() == QMediaPlaylist::Random) {
		_playlist->shuffle(trackIndex);
	} else {
		_playlist->setCurrentIndex(trackIndex);
	}
}*/

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
	// Everything is splitted in 2: local actions and remote actions
	qDebug() << Q_FUNC_INFO << mc.canonicalUrl() << mc.canonicalUrl().isLocalFile();
	if (mc.canonicalUrl().isLocalFile()) {
		_localPlayer->play(mc.canonicalUrl().toLocalFile());
	} else if (_remotePlayer) {
		_remotePlayer->play(mc.canonicalUrl());
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
	if (!_playlist || (_playlist && _playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->previousIndex() < 0)) {
		return;
	}
	_state = QMediaPlayer::StoppedState;
	_playlist->skipBackward();
	this->play();
}

void MediaPlayer::skipForward()
{
	if (!_playlist || (_playlist && _playlist->playbackMode() == QMediaPlaylist::Sequential && _playlist->nextIndex() < _playlist->currentIndex())) {
		//if (_state != QMediaPlayer::StoppedState) {
		//	this->stop();
		//}
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
	qDebug() << Q_FUNC_INFO;
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
	qDebug() << Q_FUNC_INFO << "about to stop";
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
		qDebug() << Q_FUNC_INFO << "about to resume";
		this->resume();
		break;
	case QMediaPlayer::StoppedState:
		qDebug() << Q_FUNC_INFO << "about to play";
		this->play();
		break;
	case QMediaPlayer::PlayingState:
		qDebug() << Q_FUNC_INFO << "about to pause";
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
