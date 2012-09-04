#include <QtGui/QApplication>

#include "mainwindow.h"

#define SOFT "MmeMiamMiamMusicPlayer"
#define COMPANY "MmeMiamMiam"
#define VERSION "0.3.0"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);

	MainWindow *window = new MainWindow();
#ifdef Q_OS_DARWIN
	window->setWindowIcon(QIcon(":/icons/mmmmp_osx"));
#endif
#ifdef Q_OS_WIN32
	window->setWindowIcon(QIcon(":/icons/mmmmp_win32"));
#endif
	window->show();

	return app.exec();
}
