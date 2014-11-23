#include <QApplication>
#include <QSharedMemory>

#include "mainwindow.h"

#define COMPANY "MmeMiamMiam"
#define SOFT "MiamPlayer"
#define VERSION "0.7.1"

#include "miamstyle.h"
#include "plugininfo.h"
#include <qtsingleapplication/QtSingleApplication>

#include "debug/logbrowser.h"
#include <QPointer>
QPointer<LogBrowser> logBrowser;

void debugOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
	if (logBrowser) {
		logBrowser->outputMessage(type, msg);
	}
}

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(mp);

	qRegisterMetaType<GenericDAO>();
	qRegisterMetaType<TrackDAO>();
	qRegisterMetaTypeStreamOperators<TrackDAO>("TrackDAO");
	qRegisterMetaType<QFileInfo>();
	qRegisterMetaType<PluginInfo>();
	qRegisterMetaTypeStreamOperators<PluginInfo>("PluginInfo");
	qInstallMessageHandler(debugOutput);

	QtSingleApplication app(SOFT, argc, argv);

	if (app.isRunning()) {
		QString arg = QApplication::arguments().join(";");
		app.sendMessage(arg);
		return 0;
	}

	app.setStyle(new MiamStyle);
	MainWindow *window = new MainWindow;
	logBrowser = new LogBrowser;
	QObject::connect(window->actionShowDebug, &QAction::triggered, [=]() {
		logBrowser->show();
	});
	QObject::connect(&app, &QtSingleApplication::sendArgs, window, &MainWindow::processArgs);
	app.setActivationWindow(window);

	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->isCustomColors()) {
		app.setPalette(settings->value("customPalette").value<QPalette>());
	}
	app.setOrganizationName(COMPANY);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);

	window->init();
	window->show();
	window->loadPlugins();

	// It this application was started from a file (for example)
	if (argc > 1) {
		QStringList args;
		for (int i = 0; i < argc; i++) {
			args << argv[i];
		}
		window->processArgs(args);
	}
	int result = app.exec();
	delete logBrowser;
	return result;
}
