#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>
#include "mediaplaylist.h"

#include "miamcore_global.h"

class RemoteMediaPlayer;

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;
struct libvlc_media_t;

/**
 * \brief The MediaPlayer class is a central class which controls local and remote sources.
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

	VlcInstance *_instance;
	VlcMedia *_media;
	VlcMediaPlayer *_player;
	RemoteMediaPlayer *_remotePlayer;

	QMap<QString, RemoteMediaPlayer*> _remotePlayers;
	bool _stopAfterCurrent;

	/** The unique instance of this class. */
	static MediaPlayer *_mediaPlayer;

	explicit MediaPlayer(QObject *parent = 0);
public:
	static MediaPlayer *instance();

	void addRemotePlayer(RemoteMediaPlayer *remotePlayer);

	void changeTrack(const QMediaContent &mediaContent);

	void changeTrack(MediaPlaylist *playlist, int trackIndex);

	inline bool isStopAfterCurrent() const { return _stopAfterCurrent; }

	inline MediaPlaylist * playlist() { return _playlist; }

	void seek(float pos);

	/** Set mute on or off. */
	void setMute(bool b) const;

	inline void setPlaylist(MediaPlaylist *playlist) { _playlist = playlist; }

	void setState(QMediaPlayer::State state);

	void setTime(qint64 t) const;

	void setVolume(int v);

	inline QMediaPlayer::State state() const { return _state; }

	qint64 time() const;

	inline VlcMediaPlayer *vlcMediaPlayer() const { return _player; }

private:
	void createLocalConnections();

	void createRemoteConnections(const QUrl &track);

	/** Current duration of the media, in ms. */
	qint64 duration();

	/** Play track directly in the player, without playlist. */
	void playMediaContent(const QMediaContent &mc);

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

private slots:
	void convertMedia(libvlc_media_t *);

signals:
	void currentMediaChanged(const QString &uri);
	void mediaStatusChanged(QMediaPlayer::MediaStatus);
	void positionChanged(qint64 pos, qint64 duration);
	void stateChanged(QMediaPlayer::State);
};

#endif // MEDIAPLAYER_H
