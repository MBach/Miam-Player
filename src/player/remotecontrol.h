#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <abstractview.h>

#include <QWebSocketServer>
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
	AbstractView *_currentView;

	int _port;

	//QTcpServer *_tcpServer;
	//QTcpSocket *_tcpSocket;
	QWebSocketServer *_webSocketServer;
	QWebSocket *_webSocket;

	QUdpSocket *_udpSocket;
	QTimer *_timer;

public:
	enum Command : int {	CMD_Playback		= 0,
							CMD_State			= 1,
							CMD_Track			= 2,
							CMD_Volume			= 3,
							CMD_Connection		= 4,
							CMD_Cover			= 5,
							CMD_Position		= 6,
							CMD_ActivePlaylists	= 7,
							CMD_AllPlaylists	= 8,
							CMD_LoadActivePlaylist	= 9};

	explicit RemoteControl(AbstractView *currentView, int port, QObject *parent = 0);

	virtual ~RemoteControl();

	void changeServerPort(int port);

	void startServer();

private slots:
	void decodeResponseFromClient(const QString &message);

	void initializeConnection();

	void mediaPlayerStatedChanged(QMediaPlayer::State state);

	void sendActivePlaylist(int index);

	void sendActivePlaylists() const;

	void sendAllPlaylists() const;

	void sendPosition(qint64 pos, qint64 duration);

	void sendTrackInfos(const QString &track);

	void sendVolume(qreal volume);
};

#endif // REMOTECONTROL_H
