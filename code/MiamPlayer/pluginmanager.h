#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <interfaces/mediaplayerplugininterface.h>
#include <QDir>

#include <QTableWidgetItem>

class MainWindow;

class PluginManager : public QObject
{
	Q_OBJECT
private:
	MainWindow *_mainWindow;
	QMap<QString, QFileInfo> _plugins;
	QMap<QString, BasicPluginInterface*> _instances;
	QMap<QString, QObjectList> _dependencies;

public:
	explicit PluginManager(MainWindow *mainWindow);

	void init(const QDir &appDirPath);

private:
	void loadPlugin(const QFileInfo &pluginFileInfo);

	void unloadPlugin(const QString &pluginName);

private slots:
	void loadOrUnload(QTableWidgetItem *item);
};

#endif // PLUGINMANAGER_H
