#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>
#include <QMediaPlaylist>

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
	QMediaPlaylist *_playlist;
	QMediaPlayer::State _state;

	VlcInstance *_instance;
	VlcMedia *_media;
	VlcMediaPlayer *_player;

	QMap<QString, RemoteMediaPlayer*> _remotePlayers;

public:
	explicit MediaPlayer(QObject *parent = 0);

	void addRemotePlayer(RemoteMediaPlayer *remotePlayer);

	QMediaPlaylist * playlist();
	void setPlaylist(QMediaPlaylist *playlist);

	void setVolume(int v);
	int volume() const;

	qint64 duration();

	QMediaPlayer::State state() const;
	void setState(QMediaPlayer::State state);

	void setMute(bool b) const;

	void seek(float pos);

private:
	void createLocalConnections();

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

	/** Activate or desactive audio output. */
	void toggleMute() const;

protected:
	RemoteMediaPlayer * remoteMediaPlayer(const QUrl &track, bool autoConnect = true);

private slots:
	void convertMedia(libvlc_media_t *);
	void disconnectPlayers(bool isLocal);

signals:
	void currentMediaChanged(const QMediaContent &);
	void mediaStatusChanged(QMediaPlayer::MediaStatus);
	void positionChanged(qint64 pos, qint64 duration);
	void stateChanged(QMediaPlayer::State);
};

#endif // MEDIAPLAYER_H
