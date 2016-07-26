#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "mediaplayer.h"

#include <QTcpServer>
#include <QUdpSocket>

/**
 * \brief		The RemoteControl class is a class which allows client-side applications (like App on SmartPhones) to control the player.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class RemoteControl : public QObject
{
	Q_OBJECT
	Q_ENUMS(Command)

private:
	MediaPlayer *_mediaPlayer;

	int _port;

	QTcpServer *_tcpServer;
	QTcpSocket *_tcpSocket;
	QUdpSocket *_udpSocket;

public:
	enum Command : int {	CMD_Playback		= 0,
							CMD_State			= 1,
							CMD_Track			= 2,
							CMD_Volume			= 3,
							CMD_Connection		= 4,
							CMD_Cover			= 5,
							CMD_ActivePlaylists	= 7,
							CMD_AllPlaylists	= 6};

	explicit RemoteControl(MediaPlayer *mediaPlayer, int port, QObject *parent = 0);

	virtual ~RemoteControl();

	void changeServerPort(int port);

	void startServer();

private slots:
	void decodeResponseFromClient();

	void initializeConnection();

	void mediaPlayerStatedChanged(QMediaPlayer::State state);

	void sendActivePlaylists() const;

	void sendAllPlaylists() const;

	void sendTrackInfos(const QString &track);

	void sendVolume(qreal volume);
};

#endif // REMOTECONTROL_H
