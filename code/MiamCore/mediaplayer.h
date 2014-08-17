#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>
#include <QMediaPlaylist>

#include "miamcore_global.h"
#include "remotemediaplayer.h"

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;
struct libvlc_media_t;

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

	void addRemotePlayer(const QString &id, RemoteMediaPlayer *remotePlayer) { _remotePlayers.insert(id, remotePlayer); }

	QMediaPlaylist * playlist();
	void setPlaylist(QMediaPlaylist *playlist);

	void setVolume(int v);
	int volume() const;

	qint64 duration();

	QMediaPlayer::State state() const;
	void setState(QMediaPlayer::State state);

	void setMute(bool b) const;

	void seek(float pos);

public slots:
	/** Pause current playing track. */
	void pause();

	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track. */
	void skipBackward();

	/** Change the current track. */
	void skipForward();

	/** Play current track in the playlist. */
	void play();

	/** Stop current track in the playlist. */
	void stop();

	/** Activate or desactive audio output. */
	void toggleMute() const;

private slots:
	void convertMedia(libvlc_media_t *);

signals:
	void currentMediaChanged(const QMediaContent &);
	void mediaStatusChanged(QMediaPlayer::MediaStatus);
	void positionChanged(qint64 pos, qint64 duration);
	void stateChanged(QMediaPlayer::State);

	/// XXX: test
	void aboutToPauseRemoteWebPlayer();
	void aboutToPlayRemoteWebPlayer(const QUrl &track);
	void aboutToResumeRemoteWebPlayer(const QUrl &track);
	void aboutToSeekRemoteWebPlayer(float position);
	void aboutToStopWebPlayer();
	void remotePositionChanged(qint64 pos, qint64 duration);
	void setVolumeRemote(int v);
};

#endif // MEDIAPLAYER_H
