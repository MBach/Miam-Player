#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

SqlDatabase::SqlDatabase()
	: QSqlDatabase("QSQLITE")
{
	QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
	QString dbPath = QDir::toNativeSeparators(path + "/mmmmp.db");
	QDir userDataPath(path);
	if (!userDataPath.exists(path)) {
		userDataPath.mkpath(path);
	}
	setDatabaseName(dbPath);

	if (open()) {
		exec("CREATE TABLE IF NOT EXISTS playlists (absPath varchar(255), name varchar(255), hash varchar(255))");
		exec("CREATE INDEX indexPlaylists ON playlists (absPath)");
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
