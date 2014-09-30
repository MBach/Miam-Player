#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QSqlError>
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
		exec("CREATE TABLE IF NOT EXISTS playlists (absPath varchar(255), id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, checksum varchar(255))");
		exec("CREATE TABLE IF NOT EXISTS playlistTracks (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, artist varchar(255), album varchar(255), playlistId INTEGER, FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		exec("CREATE INDEX IF NOT EXISTS indexPlaylists ON playlists (id)");
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

bool SqlDatabase::insertIntoTablePlaylistTracks(int playlistId, const std::list<RemoteTrack> &tracks)
{
	if (!isOpen()) {
		open();
	}

	exec("BEGIN TRANSACTION");
	for (std::list<RemoteTrack>::const_iterator it = tracks.cbegin(); it != tracks.cend(); ++it) {
		RemoteTrack track = *it;
		QSqlQuery insert(*this);
		if (track.id().isEmpty()) {
			insert.prepare("INSERT INTO playlistTracks (id, title, duration, artist, album, playlistId) VALUES (?, ?, ?, ?, ?, ?)");
			insert.addBindValue(track.id());
			insert.addBindValue(track.title());
			insert.addBindValue(track.length());
			insert.addBindValue(track.artist());
			insert.addBindValue(track.album());
		} else {
			insert.prepare("INSERT INTO playlistTracks (absPath, title, artist, album, playlistId) VALUES (?, ?, ?, ?, ?)");
			insert.addBindValue(track.url());
			insert.addBindValue(track.title());
			insert.addBindValue(track.artist());
			insert.addBindValue(track.album());
		}
		insert.addBindValue(playlistId);
		insert.exec();
	}
	exec("COMMIT TRANSACTION");
	close();
	return lastError().type() == QSqlError::NoError;
}

int SqlDatabase::insertIntoTablePlaylists(const RemotePlaylist &playlist)
{
	qDebug() << Q_FUNC_INFO;
	if (!isOpen()) {
		open();
	}

	int id;
	if (playlist.id().isEmpty()) {
		id = qrand();
	} else {
		id = playlist.id().toInt();
	}

	QSqlQuery insert(*this);
	insert.prepare("INSERT INTO playlists(id, title, duration, checksum) VALUES (:id, :title, :length, :chk)");
	insert.bindValue(":id", id);
	insert.bindValue(":title", playlist.title());
	insert.bindValue(":length", playlist.length());
	insert.bindValue(":chk", playlist.checksum());
	bool b = insert.exec();
	int i = 0;
	while (!b && i < 10) {
		insert.bindValue(":id", QString::number(qrand()));
		b = insert.exec();
		++i;
		qDebug() << "insert failed for playlist" << playlist.title() << "trying again" << i;
	}
	close();
	return id;
}

void SqlDatabase::removePlaylists(const QList<RemotePlaylist> &playlists)
{
	if (!isOpen()) {
		open();
	}

	exec("BEGIN TRANSACTION");
	foreach (RemotePlaylist playlist, playlists) {
		QSqlQuery remove(*this);
		remove.prepare("DELETE FROM playlists WHERE id = :id");
		remove.bindValue(":id", playlist.id());
		remove.exec();
	}
	exec("COMMIT TRANSACTION");

	close();
}
