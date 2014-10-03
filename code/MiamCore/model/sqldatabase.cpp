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
		exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, checksum varchar(255))");
		exec("CREATE TABLE IF NOT EXISTS playlistTracks (trackNumber INTEGER, title varchar(255), album varchar(255), length INTEGER, " \
			 "artist varchar(255), rating INTEGER, year INTEGER, url varchar(255), id INTEGER, playlistId INTEGER, " \
			 "FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		close();
	}
}

bool SqlDatabase::insertIntoTablePlaylistTracks(int playlistId, const std::list<TrackDAO> &tracks)
{
	if (!isOpen()) {
		open();
	}

	exec("BEGIN TRANSACTION");
	for (std::list<TrackDAO>::const_iterator it = tracks.cbegin(); it != tracks.cend(); ++it) {
		TrackDAO track = *it;
		QSqlQuery insert(*this);
		insert.prepare("INSERT INTO playlistTracks (trackNumber, title, album, length, artist, rating, year, id, url, playlistId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		insert.addBindValue(track.trackNumber());
		insert.addBindValue(track.title());
		insert.addBindValue(track.album());
		insert.addBindValue(track.length());
		insert.addBindValue(track.artist());
		insert.addBindValue(track.rating());
		insert.addBindValue(track.year());
		insert.addBindValue(track.id());
		insert.addBindValue(track.url());
		insert.addBindValue(playlistId);
		insert.exec();
	}
	exec("COMMIT TRANSACTION");
	close();
	return lastError().type() == QSqlError::NoError;
}

int SqlDatabase::insertIntoTablePlaylists(const PlaylistDAO &playlist)
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

void SqlDatabase::removePlaylists(const QList<PlaylistDAO> &playlists)
{
	if (!isOpen()) {
		open();
	}

	exec("BEGIN TRANSACTION");
	foreach (PlaylistDAO playlist, playlists) {
		/// XXX: CASCADE not working?
		QSqlQuery children(*this);
		children.prepare("DELETE FROM playlistTracks WHERE playlistId = :id");
		children.bindValue(":id", playlist.id());
		children.exec();

		QSqlQuery remove(*this);
		remove.prepare("DELETE FROM playlists WHERE id = :id");
		remove.bindValue(":id", playlist.id());
		remove.exec();
	}
	exec("COMMIT TRANSACTION");

	close();
}

QList<TrackDAO> SqlDatabase::selectPlaylistTracks(int playlistID)
{
	if (!isOpen()) {
		open();
	}

	QList<TrackDAO> tracks;
	QSqlQuery results = exec("SELECT trackNumber, title, album, length, artist, rating, year, id, url FROM playlistTracks WHERE playlistId = " + QString::number(playlistID));
	while (results.next()) {
		int i = -1;
		TrackDAO track;
		QSqlRecord record = results.record();
		track.setTrackNumber(record.value(++i).toString());
		track.setTitle(record.value(++i).toString());
		track.setAlbum(record.value(++i).toString());
		track.setLength(record.value(++i).toString());
		track.setArtist(record.value(++i).toString());
		track.setRating(record.value(++i).toInt());
		track.setYear(record.value(++i).toString());
		track.setId(record.value(++i).toString());
		track.setUrl(record.value(++i).toString());
		tracks.append(track);
	}

	close();
	return tracks;
}

PlaylistDAO SqlDatabase::selectPlaylist(int playlistId)
{
	if (!isOpen()) {
		open();
	}

	PlaylistDAO playlist;
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

QList<PlaylistDAO> SqlDatabase::selectPlaylists()
{
	if (!isOpen()) {
		open();
	}

	QList<PlaylistDAO> playlists;
	QSqlQuery results = exec("SELECT title, id FROM playlists");
	while (results.next()) {
		PlaylistDAO playlist;
		playlist.setTitle(results.record().value(0).toString());
		playlist.setId(results.record().value(1).toString());
		playlists.append(playlist);
	}

	close();
	return playlists;
}
