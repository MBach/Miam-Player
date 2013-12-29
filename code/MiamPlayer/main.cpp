#include <QApplication>

#include "mainwindow.h"

#define COMPANY "MmeMiamMiam"
#define SOFT "MmeMiamMiamMusicPlayer"
#define VERSION "0.5.0"

#include "plugininfo.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(mmmmp);

	qRegisterMetaType<QFileInfo>();
	qRegisterMetaType<PluginInfo>();
	qRegisterMetaTypeStreamOperators<PluginInfo>("PluginInfo");

	QApplication app(argc, argv);
	app.setOrganizationName(COMPANY);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);

	MainWindow window;
#ifdef Q_OS_DARWIN
	window.setWindowIcon(QIcon(":/icons/mmmmp_osx"));
#endif
#ifdef Q_OS_WIN
	window.setWindowIcon(QIcon(":/icons/mmmmp_win32"));
#endif
	window.init();
	window.show();
	window.loadPlugins();

	return app.exec();
}
