#include "singleapplication.h"

/** Checks and fires up LocalServer or closes the program if another instance already exists. */
SingleApplication::SingleApplication(int argc, char *argv[])
	: QApplication(argc, argv), _mainWindow(NULL)
{
	// By default this is not the main process
	_shouldContinue = false;

	socket = new QLocalSocket();

	// Attempt to connect to the LocalServer
	socket->connectToServer("MIAM_SERVER");
	if (socket->waitForConnected(100)) {
		socket->write("CMD:showUp");
		QString command = "|CMD:processArgs;" + QApplication::arguments().join(";");
		socket->write(command.toStdString().data());
		socket->flush();
		QThread::msleep(100);
		socket->close();
	} else {
		// The attempt was insuccessful, so we continue the program
		_shouldContinue = true;
		server = new LocalServer();
		server->start();


	}
}

SingleApplication::~SingleApplication()
{
	if (_shouldContinue) {
		server->terminate();
	}
}

/** Weather the program should be terminated. */
bool SingleApplication::shouldContinue()
{
	return _shouldContinue;
}

void SingleApplication::setActivationWindow(MainWindow *mainWindow)
{
	_mainWindow = mainWindow;

	connect(server, &LocalServer::showUp, this, [=]() {
		_mainWindow->activateWindow();
	});
	connect(server, &LocalServer::aboutToTransferArgs, _mainWindow, &MainWindow::processArgs);
}

