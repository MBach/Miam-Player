#include <QtGui/QApplication>

#include "mainwindow.h"

#define SOFT "MmeMiamMiamMusicPlayer"
#define COMPANY "MmeMiamMiam"
#define VERSION "0.2.3"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);

	MainWindow *window = new MainWindow();
	window->show();

	return app.exec();
}
