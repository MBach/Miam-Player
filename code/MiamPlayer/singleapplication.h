#ifndef APPLICATION_H
#define APPLICATION_H

#include "localserver.h"

#include <QApplication>
#include <QLocalSocket>

#include "mainwindow.h"

/**
 * \brief The Application class handles trivial application initialization procedures.
 */
class SingleApplication : public QApplication
{
	Q_OBJECT
private:
	QLocalSocket* socket;
	LocalServer* server;
	bool _shouldContinue;
	MainWindow *_mainWindow;

public:
	/** Checks and fires up LocalServer or closes the program if another instance already exists. */
	explicit SingleApplication(int, char *[]);

	~SingleApplication();

	bool shouldContinue();

	void setActivationWindow(MainWindow *mainWindow);

signals:
	void showMainWindow();
};

#endif // APPLICATION_H
