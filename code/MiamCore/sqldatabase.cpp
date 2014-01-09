#include "sqldatabase.h"

#include <QDir>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

#include <QSqlRecord>

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
		close();
	}
	open();
	QSqlQuery qLoadFileDB = exec("SELECT * FROM playlists");
	while (qLoadFileDB.next()) {
		qDebug() << "SqlDatabase::SqlDatabase() saved playlist reference" << qLoadFileDB.record().value(0).toString() << qLoadFileDB.record().value(1).toString() << qLoadFileDB.record().value(2).toString();
	}
}
