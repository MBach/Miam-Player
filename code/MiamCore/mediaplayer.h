#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>

#include "miamcore_global.h"

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;
struct libvlc_media_t;

#include <QMediaPlaylist>

class MIAMCORE_LIBRARY MediaPlayer : public QObject
{
	Q_OBJECT
private:
	VlcInstance *_instance;
	VlcMedia *_media;
	VlcMediaPlayer *_player;

	QMediaPlaylist *_playlist;

	QMediaPlayer::State _state;

public:
	explicit MediaPlayer(QObject *parent = 0);

	QMediaPlaylist * playlist();

	void setPlaylist(QMediaPlaylist *playlist);

	void setVolume(int v);

	qint64 duration();

	QMediaPlayer::State state() const;

	void setMute(bool b) const;

	void setPosition(float pos);

	int volume() const;

public slots:
	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track. */
	void skipBackward();

	/** Change the current track. */
	void skipForward();

	void pause();
	void play();
	void stop();

	void toggleMute() const;

private slots:
	void convertMedia(libvlc_media_t *);

signals:
	void currentMediaChanged(const QMediaContent &);
	void mediaStatusChanged(QMediaPlayer::MediaStatus);
	void positionChanged(qint64 pos);
	void stateChanged(QMediaPlayer::State);

	void pauseRemote();
	void playRemote(const QUrl &track);
	void setVolumeRemote(int v);
};

#endif // MEDIAPLAYER_H
