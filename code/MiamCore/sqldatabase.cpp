#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

#include "settingsprivate.h"

SqlDatabase::SqlDatabase()
	: QSqlDatabase("QSQLITE")
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	QString path("%1/%2/%3");
	path = path.arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation),
					settings->organizationName(),
					settings->applicationName());
	QString dbPath = QDir::toNativeSeparators(path + "/mp.db");
	QDir userDataPath(path);
	// Init a new database file for settings
	QFile db(dbPath);
	if (!userDataPath.exists(path)) {
		userDataPath.mkpath(path);
	}
	db.open(QIODevice::ReadWrite);
	db.close();
	setDatabaseName(dbPath);

	if (open()) {
		exec("CREATE TABLE IF NOT EXISTS playlists (absPath varchar(255), name varchar(255), hash varchar(255))");
		exec("CREATE INDEX IF NOT EXISTS indexPlaylists ON playlists (absPath)");
		close();
	}
}

/** Resynchronize table Playlists in case one has deleted some files. */
void SqlDatabase::cleanBeforeQuit()
{
	if (!open()) {
		open();
	}
	QSqlQuery qLoadFileDB = exec("SELECT absPath FROM playlists");
	QStringList deletedPlaylists;
	while (qLoadFileDB.next()) {
		qDebug() << "saved playlist reference" << qLoadFileDB.record().value(0).toString();
		QString playlist = qLoadFileDB.record().value(0).toString();
		if (!QFile::exists(playlist)) {
			deletedPlaylists << QString("'" + playlist + "'");
		}
	}
	if (!deletedPlaylists.isEmpty()) {
		exec("DELETE FROM playlists WHERE absPath IN (" + deletedPlaylists.join(',') + ")");
	}
	close();
}
