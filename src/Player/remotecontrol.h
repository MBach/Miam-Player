#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QTcpServer>

/**
 * \brief		The RemoteControl class is a class which allows client-side applications (like App on SmartPhones) to control the player.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class RemoteControl : public QObject
{
	Q_OBJECT
private:
	int _port;

	QTcpServer *_tcpServer;

public:
	explicit RemoteControl(int port, QObject *parent = 0);

	virtual ~RemoteControl();

	void startServer();

public slots:
	void changeServerPort(int port);

	void sendWelcomeToClient();

};

#endif // REMOTECONTROL_H
