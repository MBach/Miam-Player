#include "sqldatabase.h"

#include <QDir>
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
		exec("CREATE TABLE IF NOT EXISTS playlists (absPath varchar(255), name varchar(255), lastModified datetime, hash varchar(255))");
		close();
	}
}
