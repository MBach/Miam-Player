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

	qDebug() << "dbPath" << dbPath;


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
	QSqlQuery results(*this);
	results.prepare("SELECT trackNumber, title, album, length, artist, rating, year, icon, id, url FROM playlistTracks WHERE playlistId = ?");
	results.addBindValue(playlistID);
	if (results.exec()) {
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

void SqlDatabase::updateTableAlbumWithCoverImage(int albumId, const QString &coverPath)
{
	if (!isOpen()) {
		open();
	}

	/// XXX: split table tracks and create "tracks", "artists" and "albums"
	/*QSqlQuery update(*this);
	update.prepare("UPDATE tracks SET cover = ? WHERE id = ?");
	update.addBindValue(backgroundImagePath);
	update.addBindValue(playlistID);
	update.exec();*/

	qDebug() << Q_FUNC_INFO << "not yet implemented";

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
			updateTrack.prepare("UPDATE tracks SET album = ?, artist = ?, artistAlbum = ?, disc = ?, internalCover = ?, title = ?, trackNumber = ?, year = ? WHERE uri = ?");
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

	qDebug() << Q_FUNC_INFO;
	switch (SettingsPrivate::getInstance()->value("insertPolicy").toInt()) {
	case IP_Artists: {

		// Level 1: Artists
		QSqlQuery qArtists("SELECT name FROM artists", *this);
		if (qArtists.exec()) {
			while (qArtists.next()) {
				QSqlRecord record = qArtists.record();
				TrackDAO artistDAO;
				QString artist = record.value(0).toString();
				qDebug() << "artist" << artist;
				artistDAO.setTitle(artist);
				// emit artistExtracted(artistDAO);
				//void LibraryTreeView::insertNode(GenericDAO *parent, GenericDAO *node)
				emit nodeExtracted(&artistDAO);

				// Level 2: Albums
				QSqlQuery qAlbums(*this);
				qAlbums.prepare("SELECT name, year, cover FROM albums WHERE artist = ?");
				qAlbums.addBindValue(artist);
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO albumDAO;
						QString album = r.value(0).toString();
						qDebug() << "album" << album;
						albumDAO.setTitle(album);
						albumDAO.setYear(r.value(1).toString());
						albumDAO.setIconPath(r.value(2).toString());

						// emit albumExtracted(albumDAO);
						emit nodeExtracted(&albumDAO, 1, artist);

						// Level 3: Tracks
						QSqlQuery qTracks(*this);
						qTracks.prepare("SELECT uri, trackNumber, title, artist, album, artistAlbum, length, rating, disc, internalCover FROM tracks WHERE artist = ? AND album = ?");
						qTracks.addBindValue(artist);
						qTracks.addBindValue(album);
						if (qTracks.exec()) {
							while (qTracks.next()) {

								QSqlRecord r = qTracks.record();
								TrackDAO trackDAO;
								int i = -1;
								trackDAO.setUri(r.value(++i).toString());
								trackDAO.setTrackNumber(r.value(++i).toString());
								trackDAO.setTitle(r.value(++i).toString());
								qDebug() << "track" << r.value(i).toString();
								trackDAO.setArtist(r.value(++i).toString());
								trackDAO.setAlbum(r.value(++i).toString());
								trackDAO.setArtistAlbum(r.value(++i).toString());
								trackDAO.setLength(r.value(++i).toString());
								trackDAO.setRating(r.value(++i).toInt());
								trackDAO.setDisc(r.value(++i).toString());
								//emit trackExtracted2(trackDAO);
								emit nodeExtracted(&trackDAO, 2, album);
							}
						}
					}
				}
			}
		}
	}
	case IP_Albums: {

	}
	case IP_ArtistsAlbums: {

	}
	case IP_Years: {

	}
	}

	emit loaded();
	close();
}

/** Safe delete and recreate from scratch (table Tracks only). */
void SqlDatabase::rebuild()
{
	emit aboutToLoad();

	// Open Connection
	open();
	QSqlQuery createDb(*this);
	createDb.exec("PRAGMA foreign_keys = ON");
	createDb.exec("PRAGMA foreign_keys = ON");
	createDb.exec("PRAGMA synchronous = OFF");
	createDb.exec("PRAGMA journal_mode = MEMORY");
	createDb.exec("DROP TABLE artists");
	createDb.exec("DROP TABLE albums");
	createDb.exec("DROP TABLE tracks");
	createDb.exec("CREATE TABLE IF NOT EXISTS artists (id INTEGER, name varchar(255), UNIQUE(name))");
	createDb.exec("CREATE TABLE IF NOT EXISTS albums (id INTEGER, name varchar(255), year INTEGER, cover varchar(255), artist varchar(255), UNIQUE(name, artist))");
	QString createTableTracks = "CREATE TABLE IF NOT EXISTS tracks (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, " \
		"title varchar(255), artist varchar(255), album varchar(255), artistAlbum varchar(255), length INTEGER, " \
		"rating INTEGER, disc INTEGER, internalCover INTEGER DEFAULT 0)";
	createDb.exec(createTableTracks);

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
	QSqlQuery updateCoverPath("UPDATE albums SET cover = ? WHERE path = ?", *this);
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
		emit trackExtracted2(track);
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
		QSqlQuery insertTrack(*this);

		insertTrack.prepare("INSERT INTO tracks (uri, trackNumber, title, artist, album, artistAlbum, length, rating, disc, internalCover) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		insertTrack.addBindValue("file://" + absFilePath);
		track.setUri("file://" + absFilePath);

		QString tn = fh.trackNumber();
		insertTrack.addBindValue(tn.toInt());
		track.setTrackNumber(tn);

		QString title = fh.title();
		insertTrack.addBindValue(title);
		track.setTitle(title);

		QString artist = fh.artist();
		insertTrack.addBindValue(artist);
		track.setArtist(artist);

		QString album = fh.album();
		insertTrack.addBindValue(album);
		track.setAlbum(album);

		QString artistAlbum = fh.artistAlbum();
		insertTrack.addBindValue(artistAlbum);
		track.setArtistAlbum(artistAlbum);

		QString length = fh.length();
		insertTrack.addBindValue(length);
		track.setLength(length);

		int rating = fh.rating();
		insertTrack.addBindValue(rating);
		track.setRating(rating);

		int dn = fh.discNumber();
		insertTrack.addBindValue(dn);
		track.setDisc(QString::number(dn));

		insertTrack.addBindValue(fh.hasCover());

		if (insertTrack.exec()) {
			QSqlQuery insertAlbum(*this);
			insertAlbum.prepare("INSERT OR IGNORE INTO albums (name, year, artist) VALUES (?, ?, ?)");
			insertAlbum.addBindValue(fh.album());
			insertAlbum.addBindValue(fh.year());
			QString artist = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();
			insertAlbum.addBindValue(artist);
			insertAlbum.exec();

			QSqlQuery insertArtist(*this);
			insertArtist.prepare("INSERT OR IGNORE INTO artists (name) VALUES (?)");
			insertArtist.addBindValue(artist);
			insertArtist.exec();
		}
		emit trackExtracted2(track);
	} else {
		qDebug() << "INVALID FILE FOR:" << absFilePath;
	}
}
