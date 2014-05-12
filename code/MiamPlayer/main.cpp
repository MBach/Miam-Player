#include <QApplication>
#include <QSharedMemory>

#include "mainwindow.h"

#define COMPANY "MmeMiamMiam"
#define SOFT "MiamPlayer"
#define VERSION "0.6.3"

#include "plugininfo.h"

#include "miamstyle.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(mmmmp);

	qRegisterMetaType<QFileInfo>();
	qRegisterMetaType<PluginInfo>();
	qRegisterMetaTypeStreamOperators<PluginInfo>("PluginInfo");

	QApplication app(argc, argv);
	app.setStyle(new MiamStyle());
	MainWindow window;

	QSharedMemory sharedMemory;
	sharedMemory.setKey("MIAMPLAYER");
	sharedMemory.attach();

	qDebug() << argc;
	for (int i = 0; i < argc; i++) {
		qDebug() << i << argv[i];
	}

	// Exit already a process running
	if (!sharedMemory.create(1)) {
		return 0;
	}

	Settings *settings = Settings::getInstance();
	if (settings->isCustomColors()) {
		app.setPalette(settings->value("customPalette").value<QPalette>());
	}
	app.setOrganizationName(COMPANY);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);

	window.init();
	window.show();
	window.loadPlugins();

	return app.exec();
}
