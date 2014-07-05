#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QThread>
#include <QVector>
#include <QLocalServer>
#include <QLocalSocket>

class LocalServer : public QThread
{
	Q_OBJECT
private:
	QLocalServer* server;
	QVector<QLocalSocket*> clients;

public:
	LocalServer();
	~LocalServer();

protected:
	void run();
	void exec();

private slots:
	void slotNewConnection();
	void slotOnData(QString data);

signals:
	void dataReceived(QString data);
	void privateDataReceived(QString data);
	void aboutToTransferArgs(const QStringList &args);
	void showUp();
};

#endif // LOCALSERVER_H
