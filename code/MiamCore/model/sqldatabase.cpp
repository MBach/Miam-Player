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
		exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER, title varchar(255), duration INTEGER, checksum varchar(255))");
		exec("CREATE TABLE IF NOT EXISTS playlistTracks (url varchar(255), host varchar(255), id INTEGER, title varchar(255), duration INTEGER, artist varchar(255), album varchar(255), playlistId INTEGER, FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		close();
	}
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
			insert.prepare("INSERT INTO playlistTracks (url, id, title, artist, album, playlistId) VALUES (?, ?, ?, ?, ?, ?)");
			insert.addBindValue(track.url());
			insert.addBindValue(qHash(track.url()));
		} else {
			insert.prepare("INSERT INTO playlistTracks (url, id, duration, title, artist, album, playlistId) VALUES (?, ?, ?, ?, ?, ?, ?)");
			insert.addBindValue(track.url());
			insert.addBindValue(track.id());
			insert.addBindValue(track.length());
		}
		insert.addBindValue(track.title());
		insert.addBindValue(track.artist());
		insert.addBindValue(track.album());
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

QList<RemoteTrack> SqlDatabase::selectPlaylistTracks(int playlistID)
{
	if (!isOpen()) {
		open();
	}

	QList<RemoteTrack> tracks;
	QSqlQuery results = exec("SELECT url, id, title, duration, artist, album FROM playlistTracks WHERE playlistId = " + QString::number(playlistID));
	while (results.next()) {
		int i = -1;
		RemoteTrack track;
		track.setUrl(results.record().value(++i).toString());
		track.setId(results.record().value(++i).toString());
		track.setTitle(results.record().value(++i).toString());
		track.setLength(results.record().value(++i).toString());
		track.setArtist(results.record().value(++i).toString());
		track.setAlbum(results.record().value(++i).toString());
		tracks.append(track);
	}

	close();
	return tracks;
}

RemotePlaylist SqlDatabase::selectPlaylist(int playlistId)
{
	if (!isOpen()) {
		open();
	}

	RemotePlaylist playlist;
	QSqlQuery results = exec("SELECT id, title, checksum FROM playlists WHERE id = " + QString::number(playlistId));
	if (results.next()) {
		int i = -1;
		playlist.setId(results.record().value(++i).toString());
		playlist.setTitle(results.record().value(++i).toString());
		playlist.setChecksum(results.record().value(++i).toString());
	}

	close();
	return playlist;
}

QList<RemotePlaylist> SqlDatabase::selectPlaylists()
{
	if (!isOpen()) {
		open();
	}

	QList<RemotePlaylist> playlists;
	QSqlQuery results = exec("SELECT title, id FROM playlists");
	while (results.next()) {
		RemotePlaylist playlist;
		playlist.setTitle(results.record().value(0).toString());
		playlist.setId(results.record().value(1).toString());
		playlists.append(playlist);
	}

	close();
	return playlists;
}
