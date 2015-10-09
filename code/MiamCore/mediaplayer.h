#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>
#include "mediaplaylist.h"
#include "miamcore_global.h"

class IMediaPlayer;

namespace QtAV {
	class AVPlayer;
}

/**
 * \brief		The MediaPlayer class is a central class which controls local and remote sources.
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaPlayer : public QObject
{
	Q_OBJECT
private:
	MediaPlaylist *_playlist;
	QMediaPlayer::State _state;

	QtAV::AVPlayer *_localPlayer;
	IMediaPlayer *_remotePlayer;

	QMap<QString, IMediaPlayer*> _remotePlayers;
	bool _stopAfterCurrent;

public:
	explicit MediaPlayer(QObject *parent = 0);

	void addRemotePlayer(IMediaPlayer *remotePlayer);

	void changeTrack(const QMediaContent &mediaContent);

	void changeTrack(MediaPlaylist *playlist, int trackIndex);

	inline bool isStopAfterCurrent() const { return _stopAfterCurrent; }

	inline MediaPlaylist * playlist() { return _playlist; }

	void seek(qreal pos);

	/** Set mute on or off. */
	void setMute(bool b) const;

	inline void setPlaylist(MediaPlaylist *playlist) { _playlist = playlist; }

	void setState(QMediaPlayer::State state);

	//void setTime(qint64 t) const;

	void setVolume(qreal v);

	inline QMediaPlayer::State state() const { return _state; }

	//qint64 time() const;

	/** Play track directly in the player, without playlist. */
	void playMediaContent(const QMediaContent &mc);

private:
	//void createLocalConnections();

	//void createRemoteConnections(const QUrl &track);

	/** Current duration of the media, in ms. */
	qint64 duration();

	/** Current position in the media, percent-based. */
	float position() const;

public slots:
	/** Pause current playing track. */
	void pause();

	/** Play current track in the playlist. */
	void play();

	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track. */
	void skipBackward();

	/** Change the current track. */
	void skipForward();

	/** Stop current track in the playlist. */
	void stop();

	inline void stopAfterCurrent(bool b) { _stopAfterCurrent = b; }

	/** Activate or desactive audio output. */
	void toggleMute() const;

signals:
	void currentMediaChanged(const QString &uri);
	void mediaStatusChanged(QMediaPlayer::MediaStatus);
	void positionChanged(qint64 pos, qint64 duration);
	void stateChanged(QMediaPlayer::State);
};

#endif // MEDIAPLAYER_H
