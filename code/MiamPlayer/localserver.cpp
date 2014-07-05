#include "localserver.h"

#include <QFile>
#include <QStringList>

LocalServer::LocalServer()
	: QThread()
{}

LocalServer::~LocalServer()
{
	server->close();
	for(int i = 0; i < clients.size(); ++i) {
		clients[i]->close();
	}
}

void LocalServer::run()
{
	server = new QLocalServer();

	connect(server, &QLocalServer::newConnection, this, &LocalServer::slotNewConnection);
	connect(this, &LocalServer::privateDataReceived, this, &LocalServer::slotOnData);

#ifdef Q_OS_UNIX
	// Make sure the temp address file is deleted
	QFile address(QString("/tmp/MIAM_SERVER"));
	if (address.exists()) {
		address.remove();
	}
#endif

	QString serverName = QString("MIAM_SERVER");
	server->listen(serverName);
	while (server->isListening() == false) {
		server->listen(serverName);
		msleep(100);
	}
	exec();
}

void LocalServer::exec()
{
	while(server->isListening()) {
		msleep(100);
		server->waitForNewConnection(100);
		for (int i = 0; i < clients.size(); ++i) {
			if (clients[i]->waitForReadyRead(100)) {
				QByteArray data = clients[i]->readAll();
				emit privateDataReceived(data);
			}
		}
	}
}

void LocalServer::slotNewConnection()
{
	clients.push_front(server->nextPendingConnection());
}

void LocalServer::slotOnData(QString data)
{
	if (data.startsWith("CMD:", Qt::CaseInsensitive)) {
		QStringList availableSignals;
		availableSignals << "showUp" << "processArgs";

		QStringList commands = data.split("|");
		for (int i = 0; i < commands.count(); i++) {
			QStringList commandWithArgs = commands.at(i).split(";");
			QStringList args;
			int signalToExecute = -1;
			for (int j = 0; j < commandWithArgs.count(); j++) {
				if (j == 0) {
					QString commandName = commandWithArgs.at(j);
					QString sig = commandName.remove("CMD:", Qt::CaseSensitive);
					signalToExecute = availableSignals.indexOf(sig);
				} else {
					args << commandWithArgs.at(j);
				}
			}
			switch(signalToExecute) {
			case 0:
				emit showUp();
				break;
			case 1:
				emit aboutToTransferArgs(args);
				break;
			case -1:
				qDebug() << "command parsing wasn't successful :(";
				break;
			}
		}
	} else {
		emit dataReceived(data);
	}
}
