#include "remotecontrol.h"

#include <QtDebug>

RemoteControl::RemoteControl(int port, QObject *parent)
	: QObject(parent)
	, _port(port)
	, _tcpServer(nullptr)
{

}

RemoteControl::~RemoteControl()
{
	_tcpServer->close();
}

void RemoteControl::startServer()
{
	_tcpServer = new QTcpServer(this);
	bool b = _tcpServer->listen(QHostAddress::Any, _port);
	qDebug() << Q_FUNC_INFO << "is listening on port: " << _port << ". OK?" << b;
}

void RemoteControl::changeServerPort(int port)
{
	qDebug() << Q_FUNC_INFO << "closing server on port" << _port;
	_tcpServer->close();
	_port = port;
	bool b = _tcpServer->listen(QHostAddress::Any, _port);
	qDebug() << Q_FUNC_INFO << "is listening on new port" << _port << ". OK?" << b;

}
