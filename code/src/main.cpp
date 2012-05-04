#include <QtGui/QApplication>

#include "mainwindow.h"

#define SOFT "MmeMiamMiamMusicPlayer"
#define COMPANY "MmeMiamMiam"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName(SOFT);

	MainWindow *window = new MainWindow();
	window->show();

	return app.exec();
}
