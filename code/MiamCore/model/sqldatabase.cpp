#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

#include "settingsprivate.h"
#include "musicsearchengine.h"
#include "filehelper.h"

SqlDatabase::SqlDatabase(QObject *parent)
	: QObject(parent), QSqlDatabase("QSQLITE")
{
	_musicSearchEngine = new MusicSearchEngine;
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
		exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, icon varchar(255), background varchar(255), checksum varchar(255))");
		exec("CREATE TABLE IF NOT EXISTS playlistTracks (trackNumber INTEGER, title varchar(255), album varchar(255), length INTEGER, " \
			 "artist varchar(255), rating INTEGER, year INTEGER, icon varchar(255), id INTEGER, url varchar(255), playlistId INTEGER, " \
			 "FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		close();
	}


	connect(_musicSearchEngine, &MusicSearchEngine::progressChanged, this, &SqlDatabase::progressChanged);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedCover, this, &SqlDatabase::saveCoverRef);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedFile, this, &SqlDatabase::saveFileRef);

	// When the scan is complete, save the model in the filesystem
	connect(_musicSearchEngine, &MusicSearchEngine::searchHasEnded, [=] () {
		commit();
		exec("CREATE INDEX indexArtist ON tracks (artist)");
		exec("CREATE INDEX indexAlbum ON tracks (album)");
		exec("CREATE INDEX indexArtistAlbum ON tracks (artistAlbum)");
		exec("CREATE INDEX indexPath ON tracks (path)");
		// Close Connection
		database().close();
		emit loaded();
	});
}

bool SqlDatabase::insertIntoTablePlaylistTracks(int playlistId, const std::list<TrackDAO> &tracks)
{
	if (!isOpen()) {
		open();
	}

	this->transaction();
	for (std::list<TrackDAO>::const_iterator it = tracks.cbegin(); it != tracks.cend(); ++it) {
		TrackDAO track = *it;
		QSqlQuery insert(*this);
		insert.prepare("INSERT INTO playlistTracks (trackNumber, title, album, length, artist, rating, year, icon, id, url, playlistId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		insert.addBindValue(track.trackNumber());
		insert.addBindValue(track.title());
		insert.addBindValue(track.album());
		insert.addBindValue(track.length());
		insert.addBindValue(track.artist());
		insert.addBindValue(track.rating());
		insert.addBindValue(track.year());
		insert.addBindValue(track.iconPath());
		insert.addBindValue(track.id());
		insert.addBindValue(track.uri());
		insert.addBindValue(playlistId);
		insert.exec();
	}
	this->commit();
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
	insert.prepare("INSERT INTO playlists(id, title, duration, icon, checksum) VALUES (?, ?, ?, ?, ?)");
	insert.addBindValue(id);
	insert.addBindValue(playlist.title());
	insert.addBindValue(playlist.length());
	insert.addBindValue(playlist.iconPath());
	insert.addBindValue(playlist.checksum());
	bool b = insert.exec();
	int i = 0;
	while (!b && i < 10) {
		insert.bindValue(":id", QString::number(qrand()));
		b = insert.exec();
		++i;
		qDebug() << "insert failed for playlist" << playlist.title() << "trying again" << i;
		qDebug() << insert.lastError();
	}
	close();
	return id;
}

void SqlDatabase::removePlaylists(const QList<PlaylistDAO> &playlists)
{
	if (!isOpen()) {
		open();
	}

	this->transaction();
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
	this->commit();

	close();
}

QList<TrackDAO> SqlDatabase::selectPlaylistTracks(int playlistID)
{
	if (!isOpen()) {
		open();
	}

	QList<TrackDAO> tracks;
	QSqlQuery results = exec("SELECT trackNumber, title, album, length, artist, rating, year, icon, id, url FROM playlistTracks WHERE playlistId = " + QString::number(playlistID));
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
		track.setIconPath(record.value(++i).toString());
		track.setId(record.value(++i).toString());
		track.setUri(record.value(++i).toString());
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
	QSqlQuery results = exec("SELECT id, title, checksum, icon, background FROM playlists WHERE id = " + QString::number(playlistId));
	if (results.next()) {
		int i = -1;
		playlist.setId(results.record().value(++i).toString());
		playlist.setTitle(results.record().value(++i).toString());
		playlist.setChecksum(results.record().value(++i).toString());
		playlist.setIconPath(results.record().value(++i).toString());
		playlist.setBackground(results.record().value(++i).toString());
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
	QSqlQuery results = exec("SELECT title, id, icon, background FROM playlists");
	while (results.next()) {
		PlaylistDAO playlist;
		int i = -1;
		playlist.setTitle(results.record().value(++i).toString());
		playlist.setId(results.record().value(++i).toString());
		playlist.setIconPath(results.record().value(++i).toString());
		playlist.setBackground(results.record().value(++i).toString());
		playlists.append(playlist);
	}

	close();
	return playlists;
}

bool SqlDatabase::playlistHasBackgroundImage(int playlistID)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery query = exec("SELECT background FROM playlists WHERE id = " + QString::number(playlistID));
	query.next();
	bool result = !query.record().value(0).toString().isEmpty();
	qDebug() << query.record().value(0).toString() << result;

	close();
	return result;
}

void SqlDatabase::updateTablePlaylistWithBackgroundImage(int playlistID, const QString &backgroundImagePath)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery update(*this);
	update.prepare("UPDATE playlists SET background = ? WHERE id = ?");
	update.addBindValue(backgroundImagePath);
	update.addBindValue(playlistID);
	update.exec();

	close();
}

/**
 * Update a list of tracks. If track name has changed, will be removed from Library then added right after.
 * \param tracksToUpdate 'First' in pair is actual filename, 'Second' is the new filename, but may be empty.*/
void SqlDatabase::updateTracks(const QList<QPair<QString, QString>> &tracksToUpdate)
{
	if (!isOpen()) {
		open();
	}

	// Signals are blocked to prevent saveFileRef method to emit one. Load method will tell connected views to rebuild themselves
	this->blockSignals(true);
	transaction();

	for (int i = 0; i < tracksToUpdate.length(); i++) {
		QPair<QString, QString> pair = tracksToUpdate.at(i);
		// Old path, New Path. If New Path exists, then file has changed.
		if (pair.second.isEmpty()) {
			FileHelper fh(pair.first);
			QSqlQuery updateTrack(*this);
			updateTrack.prepare("UPDATE tracks SET album = ?, artist = ?, artistAlbum = ?, discNumber = ?, internalCover = ?, title = ?, trackNumber = ?, year = ? WHERE uri = ?");
			updateTrack.addBindValue(fh.album());
			updateTrack.addBindValue(fh.artist());
			updateTrack.addBindValue(fh.artistAlbum());
			updateTrack.addBindValue(fh.discNumber());
			updateTrack.addBindValue(fh.hasCover());
			updateTrack.addBindValue(fh.title());
			updateTrack.addBindValue(fh.trackNumber().toInt());
			updateTrack.addBindValue(fh.year().toInt());
			updateTrack.addBindValue(QDir::toNativeSeparators(pair.first));
			updateTrack.exec();
		} else {
			QSqlQuery hasTrack("SELECT COUNT(*) FROM tracks WHERE uri = ?", *this);
			hasTrack.addBindValue(QDir::toNativeSeparators(pair.first));
			if (hasTrack.exec() && hasTrack.next() && hasTrack.record().value(0).toInt() != 0) {
				QSqlQuery removeTrack("DELETE FROM tracks WHERE uri = ?", *this);
				removeTrack.addBindValue(QDir::toNativeSeparators(pair.first));
				qDebug() << "deleting tracks";
				if (removeTrack.exec()) {
					this->saveFileRef(pair.second);
				}
			}
		}
	}
	commit();
	this->blockSignals(false);

	close();

	/// XXX: might not be the smartest way to reload changes, but it's way simpler than searching in a tree for modifications
	this->load();
}

/** Read all tracks entries in the database and send them to connected views. */
void SqlDatabase::loadFromFileDB()
{
	emit aboutToLoad();
	if (!isOpen()) {
		open();
	}

	QSqlQuery qLoadFileDB = exec("SELECT * FROM tracks");
	while (qLoadFileDB.next()) {
		QSqlRecord r = qLoadFileDB.record();
		TrackDAO track;
		int i = -1;
		track.setArtist(r.value(++i).toString());
		track.setArtistAlbum(r.value(++i).toString());
		track.setAlbum(r.value(++i).toString());
		track.setTitle(r.value(++i).toString());
		track.setTrackNumber(r.value(++i).toString());
		track.setDisc(r.value(++i).toString());
		track.setYear(r.value(++i).toString());
		track.setUri(r.value(++i).toString());
		emit trackExtracted(track);
	}
	close();
	emit loaded();
}

/** Safe delete and recreate from scratch (table Tracks only). */
void SqlDatabase::rebuild()
{
	emit aboutToLoad();

	// Open Connection
	open();
	exec("PRAGMA foreign_keys = ON");
	exec("PRAGMA synchronous = OFF");
	exec("PRAGMA journal_mode = MEMORY");
	exec("DROP TABLE tracks");
	QString createTable = "CREATE TABLE tracks (artist varchar(255), artistAlbum varchar(255), album varchar(255), " \
		"title varchar(255), trackNumber INTEGER, discNumber INTEGER, year INTEGER, uri varchar(255) PRIMARY KEY ASC, " \
		"path varchar(255), coverAbsPath varchar(255), internalCover INTEGER DEFAULT 0)";
	exec(createTable);

	transaction();

	// Foreach file, insert tuple
	_musicSearchEngine->moveToThread(&_workerThread);
	_musicSearchEngine->doSearch();
}

/** Load an existing database file or recreate it, if not found. */
void SqlDatabase::load()
{
	if (open() && tables().contains("tracks")) {
		this->loadFromFileDB();
	} else {
		this->rebuild();
	}
}

/** Reads an external picture which is close to multimedia files (same folder). */
void SqlDatabase::saveCoverRef(const QString &coverPath)
{
	QFileInfo fileInfo(coverPath);
	QSqlQuery updateCoverPath("UPDATE tracks SET coverAbsPath = ? WHERE path = ?", *this);
	updateCoverPath.addBindValue(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
	updateCoverPath.addBindValue(QDir::toNativeSeparators(fileInfo.absolutePath()));
	updateCoverPath.exec();
	emit coverWasUpdated(fileInfo);
}

void SqlDatabase::loadRemoteTracks(const QList<TrackDAO> &tracks)
{
	qDebug() << Q_FUNC_INFO << tracks.size();
	foreach (TrackDAO track, tracks) {
		qDebug() << track.title();
		emit trackExtracted(track);
	}
}

/** Reads a file from the filesystem and adds it into the library. */
void SqlDatabase::saveFileRef(const QString &absFilePath)
{
	if (!isOpen()) {
		open();
	}
	FileHelper fh(absFilePath);
	if (fh.isValid()) {
		TrackDAO track;
		QSqlQuery insert(*this);
		insert.prepare("INSERT INTO tracks (uri, album, artist, artistAlbum, discNumber, internalCover, path, title, trackNumber, year) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		//QString nativeAbsPath = QDir::toNativeSeparators(absFilePath);
		//insert.addBindValue(nativeAbsPath);
		//track.setUri(nativeAbsPath);
		insert.addBindValue("file://" + absFilePath);
		track.setUri("file://" + absFilePath);

		QString album = fh.album();
		insert.addBindValue(album);
		track.setAlbum(album);

		QString artist = fh.artist();
		insert.addBindValue(artist);
		track.setArtist(artist);

		QString artistAlbum = fh.artistAlbum();
		insert.addBindValue(artistAlbum);
		track.setArtistAlbum(artistAlbum);

		int dn = fh.discNumber();
		insert.addBindValue(dn);
		track.setDisc(QString::number(dn));

		insert.addBindValue(fh.hasCover());
		insert.addBindValue(QDir::toNativeSeparators(fh.fileInfo().absolutePath()));

		QString title = fh.title();
		insert.addBindValue(title);
		track.setTitle(title);

		QString tn = fh.trackNumber();
		insert.addBindValue(tn.toInt());
		track.setTrackNumber(tn);

		QString year = fh.year();
		insert.addBindValue(year.toInt());
		track.setYear(year);

		insert.exec();
		emit trackExtracted(track);
	} else {
		qDebug() << "INVALID FILE FOR:" << absFilePath;
	}
}
