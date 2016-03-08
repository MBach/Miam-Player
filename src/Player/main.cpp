#include <QApplication>
#include <QSharedMemory>

#include "mainwindow.h"

#define COMPANY "MmeMiamMiam"
#define SOFT "MiamPlayer"
#define VERSION "0.8.1"

#include <settingsprivate.h>
#include "styling/miamstyle.h"
#include <plugininfo.h>
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
	Q_INIT_RESOURCE(player);

	qRegisterMetaType<GenericDAO>();
	qRegisterMetaType<TrackDAO>();
	qRegisterMetaTypeStreamOperators<TrackDAO>("TrackDAO");
	qRegisterMetaType<QFileInfo>();
	qRegisterMetaType<PluginInfo>();
	qRegisterMetaTypeStreamOperators<PluginInfo>("PluginInfo");
#if defined(Q_OS_WIN)
	qInstallMessageHandler(debugOutput);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QtSingleApplication app(SOFT, argc, argv);
	app.setOrganizationName(COMPANY);
	app.setApplicationName(SOFT);
	app.setApplicationVersion(VERSION);
	app.setAttribute(Qt::AA_UseHighDpiPixmaps);

	if (app.isRunning()) {
		app.forwardArgsToServer();
		return 0;
	}

	SettingsPrivate *settings = SettingsPrivate::instance();
	app.installTranslator(&settings->playerTranslator);
	app.installTranslator(&settings->defaultQtTranslator);

	app.setStyle(new MiamStyle);
	MainWindow *window = new MainWindow;
	app.setActivationWindow(window);

	logBrowser = new LogBrowser;
	QObject::connect(window->actionShowDebug, &QAction::triggered, [=]() { logBrowser->show(); });
	QObject::connect(&app, &QtSingleApplication::sendArgs, window, &MainWindow::processArgs);

	if (settings->isCustomColors()) {
		app.setPalette(settings->value("customPalette").value<QPalette>());
	}

	window->init();
	window->show();
	window->loadPlugins();
	window->activateLastView();
	QStringList args;
	for (int i = 0; i < argc; i++) {
		args << argv[i];
	}
	window->processArgs(args);

	int result = app.exec();
	delete logBrowser;
	return result;
}
