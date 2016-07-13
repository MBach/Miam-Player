#include "remotecontrol.h"

#include <QDataStream>
#include <QTcpSocket>

#include <QtDebug>

RemoteControl::RemoteControl(int port, QObject *parent)
	: QObject(parent)
	, _port(port)
	, _tcpServer(nullptr)
{

}

RemoteControl::~RemoteControl()
{
	qDebug() << Q_FUNC_INFO;
	_tcpServer->close();
}

void RemoteControl::startServer()
{
	_tcpServer = new QTcpServer(this);
	connect(_tcpServer, &QTcpServer::newConnection, this, &RemoteControl::sendWelcomeToClient);
	bool b = _tcpServer->listen(QHostAddress::Any, _port);
	qDebug() << Q_FUNC_INFO << "is listening on port:" << _port << ". OK?" << b;
}

void RemoteControl::changeServerPort(int port)
{
	qDebug() << Q_FUNC_INFO << "closing server on port:" << _port;
	_tcpServer->close();
	_port = port;
	bool b = _tcpServer->listen(QHostAddress::Any, _port);
	qDebug() << Q_FUNC_INFO << "is listening on new port:" << _port << ". OK?" << b;

}

void RemoteControl::sendWelcomeToClient()
{
	qDebug() << Q_FUNC_INFO;
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_6);
	out << "Hello from Miam-Player!";

	QTcpSocket *connection = _tcpServer->nextPendingConnection();
	connect(connection, &QAbstractSocket::disconnected, connection, &QObject::deleteLater);
	connection->write(block);
}
