#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

#include "cover.h"
#include "settingsprivate.h"
#include "musicsearchengine.h"
#include "filehelper.h"

SqlDatabase* SqlDatabase::_sqlDatabase = NULL;

SqlDatabase::SqlDatabase()
	: QObject(), QSqlDatabase("QSQLITE")
{
	_musicSearchEngine = new MusicSearchEngine;
	_musicSearchEngine->moveToThread(&_workerThread);

	SettingsPrivate *settings = SettingsPrivate::instance();
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
		exec("CREATE INDEX indexArtist ON tracks (artistId)");
		exec("CREATE INDEX indexAlbum ON tracks (albumId)");
		// exec("CREATE INDEX indexArtistAlbum ON tracks (artistAlbum)");
		exec("CREATE INDEX indexPath ON tracks (uri)");
		exec("CREATE INDEX indexArtistId ON artists (id)");
		exec("CREATE INDEX indexAlbumId ON albums (id)");
		// Close Connection
		database().close();
		emit loaded();
	});

	_workerThread.start();
}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
SqlDatabase* SqlDatabase::instance()
{
	if (_sqlDatabase == NULL) {
		_sqlDatabase = new SqlDatabase;
	}
	return _sqlDatabase;
}

bool SqlDatabase::insertIntoTableArtists(const ArtistDAO &artist)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery insertArtist(*this);
	insertArtist.prepare("INSERT OR IGNORE INTO artists (id, name, normalizedName) VALUES (?, ?, ?)");
	QString artistNorm = this->normalizeField(artist.title());
	uint artistId = qHash(artistNorm);

	insertArtist.addBindValue(artistId);
	insertArtist.addBindValue(artist.title());
	insertArtist.addBindValue(artistNorm);
	insertArtist.exec();

	close();
	return lastError().type() == QSqlError::NoError;
}

bool SqlDatabase::insertIntoTableAlbums(uint artistId, const AlbumDAO &album)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery insertAlbum(*this);
	insertAlbum.prepare("INSERT OR IGNORE INTO albums (id, name, normalizedName, year, artistId, host, icon) VALUES (?, ?, ?, ?, ?, ?, ?)");
	QString albumNorm = this->normalizeField(album.title());
	uint albumId = qHash(albumNorm + QString::number(artistId));

	insertAlbum.addBindValue(albumId);
	insertAlbum.addBindValue(album.title());
	insertAlbum.addBindValue(albumNorm);
	insertAlbum.addBindValue(album.year());
	insertAlbum.addBindValue(artistId);
	insertAlbum.addBindValue(album.host());
	insertAlbum.addBindValue(album.icon());
	insertAlbum.exec();

	close();
	return lastError().type() == QSqlError::NoError;
}

bool SqlDatabase::insertIntoTablePlaylistTracks(int playlistId, const std::list<TrackDAO> &tracks)
{
	qDebug() << Q_FUNC_INFO;
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
		insert.addBindValue(track.icon());
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
	insert.addBindValue(playlist.icon());
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

bool SqlDatabase::insertIntoTableTracks(const TrackDAO &track)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery insertTrack(*this);
	insertTrack.prepare("INSERT INTO tracks (uri, trackNumber, title, artistId, albumId, artistAlbum, length, rating, " \
		"disc, host, icon) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	QString artistAlbum = track.artistAlbum().isEmpty() ? track.artist() : track.artistAlbum();
	QString artistNorm = this->normalizeField(artistAlbum);
	QString albumNorm = this->normalizeField(track.album());
	uint artistId = qHash(artistNorm);
	uint albumId = qHash(albumNorm + QString::number(artistId));

	insertTrack.addBindValue(track.uri());
	insertTrack.addBindValue(track.trackNumber());
	insertTrack.addBindValue(track.title());
	insertTrack.addBindValue(artistId);
	insertTrack.addBindValue(albumId);
	insertTrack.addBindValue(track.artistAlbum());
	insertTrack.addBindValue(track.length());
	insertTrack.addBindValue(track.rating());
	insertTrack.addBindValue(track.disc());
	insertTrack.addBindValue(track.host());
	insertTrack.addBindValue(track.icon());
	bool b = insertTrack.exec();
	close();
	return b;
}

bool SqlDatabase::insertIntoTableTracks(const std::list<TrackDAO> &tracks)
{
	bool b = true;
	for (std::list<TrackDAO>::const_iterator it = tracks.cbegin(); it != tracks.cend(); ++it) {
		TrackDAO track = *it;
		bool g = this->insertIntoTableTracks(track);
		b = b && g;
	}
	return b;
}

void SqlDatabase::removeRecordsFromHost(const QString &)
{
	qDebug() << Q_FUNC_INFO;
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

Cover* SqlDatabase::selectCoverFromURI(const QString &uri)
{
	if (!isOpen()) {
		open();
	}

	Cover *c = NULL;

	QSqlQuery selectCover(*this);
	selectCover.prepare("SELECT t.internalCover, a.cover FROM albums a INNER JOIN tracks t ON a.id = t.albumId " \
		"WHERE t.uri = ?");
	selectCover.addBindValue(uri);
	if (selectCover.exec() && selectCover.next()) {
		// If URI has an internal cover, i.e. uri points to a local file
		if (selectCover.record().value(0).toBool()) {
			FileHelper fh(uri);
			c = fh.extractCover();
		} else {
			c = new Cover(selectCover.record().value(1).toString());
		}
	}

	close();
	return c;
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
			QSqlRecord record = results.record();
			TrackDAO track;
			track.setTrackNumber(record.value(++i).toString());
			track.setTitle(record.value(++i).toString());
			track.setAlbum(record.value(++i).toString());
			track.setLength(record.value(++i).toString());
			track.setArtist(record.value(++i).toString());
			track.setRating(record.value(++i).toInt());
			track.setYear(record.value(++i).toString());
			track.setIcon(record.value(++i).toString());
			track.setId(record.value(++i).toString());
			track.setUri(record.value(++i).toString());
			tracks.push_back(std::move(track));
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
		playlist.setIcon(results.record().value(++i).toString());
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
		playlist.setIcon(results.record().value(++i).toString());
		playlist.setBackground(results.record().value(++i).toString());
		playlists.append(playlist);
	}

	close();
	return playlists;
}

TrackDAO SqlDatabase::selectTrack(const QString &uri)
{
	if (!isOpen()) {
		open();
	}

	TrackDAO track;
	QSqlQuery qTracks(*this);
	qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
		"t.host, t.icon, alb.year FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
		"INNER JOIN artists art ON t.artistId = art.id " \
		"WHERE uri = ?");
	qTracks.addBindValue(uri);
	if (qTracks.exec() && qTracks.next()) {

		QSqlRecord r = qTracks.record();
		int j = -1;
		track.setUri(r.value(++j).toString());
		track.setTrackNumber(r.value(++j).toString());
		track.setTitle(r.value(++j).toString());
		track.setArtist(r.value(++j).toString());
		track.setAlbum(r.value(++j).toString());
		track.setArtistAlbum(r.value(++j).toString());
		track.setLength(r.value(++j).toString());
		track.setRating(r.value(++j).toInt());
		track.setDisc(r.value(++j).toString());
		++j;
		track.setHost(r.value(++j).toString());
		track.setIcon(r.value(++j).toString());
		track.setYear(r.value(++j).toString());
	}

	close();
	return track;
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

void SqlDatabase::updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist)
{
	if (!isOpen()) {
		open();
	}

	QSqlQuery update(*this);
	update.prepare("UPDATE albums SET cover = ? WHERE normalizedName = ? AND artistId = (SELECT id FROM artists WHERE normalizedName = ?)");
	update.addBindValue(coverPath);
	update.addBindValue(this->normalizeField(album));
	update.addBindValue(this->normalizeField(artist));
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

	/// XXX: complexity is not good!
	switch (SettingsPrivate::instance()->value("insertPolicy").toInt()) {
	case IP_Artists: {
		// Level 1: Artists
		QSqlQuery qArtists("SELECT id, name, normalizedName FROM artists", *this);
		if (qArtists.exec()) {
			while (qArtists.next()) {
				QSqlRecord record = qArtists.record();
				ArtistDAO *artistDAO = new ArtistDAO;
				uint artistId = record.value(0).toUInt();
				QString artist = record.value(1).toString();
				artistDAO->setId(record.value(0).toString());
				artistDAO->setTitle(artist);
				artistDAO->setTitleNormalized(record.value(2).toString());
				emit nodeExtracted(artistDAO);

				// Level 2: Albums
				QSqlQuery qAlbums(*this);
				qAlbums.prepare("SELECT name, normalizedName, year, cover, host, icon FROM albums WHERE artistId = ?");
				qAlbums.addBindValue(artistId);
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO *albumDAO = new AlbumDAO;
						int i = -1;
						QString album = r.value(++i).toString();
						albumDAO->setTitle(album);
						albumDAO->setTitleNormalized(r.value(++i).toString());
						QString year = r.value(++i).toString();
						albumDAO->setYear(year);
						albumDAO->setCover(r.value(++i).toString());
						albumDAO->setHost(r.value(++i).toString());
						albumDAO->setIcon(r.value(++i).toString());
						albumDAO->setParentNode(artistDAO);
						emit nodeExtracted(albumDAO);

						// Level 3: Tracks
						QSqlQuery qTracks(*this);
						qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
										"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
										"INNER JOIN artists art ON t.artistId = art.id " \
										"WHERE art.name = ? AND alb.name = ?");
						qTracks.addBindValue(artist);
						qTracks.addBindValue(album);
						if (qTracks.exec()) {
							while (qTracks.next()) {
								QSqlRecord r = qTracks.record();
								TrackDAO *trackDAO = new TrackDAO;
								int j = -1;
								QString uri = r.value(++j).toString();
								trackDAO->setUri(uri);
								trackDAO->setTrackNumber(r.value(++j).toString());
								trackDAO->setTitle(r.value(++j).toString());
								trackDAO->setArtist(r.value(++j).toString());
								trackDAO->setAlbum(r.value(++j).toString());
								trackDAO->setArtistAlbum(r.value(++j).toString());
								trackDAO->setLength(r.value(++j).toString());
								trackDAO->setRating(r.value(++j).toInt());
								trackDAO->setDisc(r.value(++j).toString());
								if (r.value(++j).toBool() && albumDAO->cover().isEmpty()) {
									albumDAO->setCover(uri);
									emit aboutToUpdateNode(albumDAO);
								}
								trackDAO->setHost(r.value(++j).toString());
								trackDAO->setIcon(r.value(++j).toString());
								trackDAO->setParentNode(albumDAO);
								trackDAO->setYear(year);
								emit nodeExtracted(trackDAO);
							}
						}
					}
				}
			}
		}
		break;
	}
	case IP_Albums: {
		qDebug() << "IP_Albums";
		break;
	}
	case IP_ArtistsAlbums: {
		qDebug() << "IP_ArtistsAlbums";
		break;
	}
	case IP_Years: {
		qDebug() << "IP_Years";
		break;
	}
	}

	emit loaded();
	close();
}

/** Safe delete and recreate from scratch. */
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
	createDb.exec("CREATE TABLE IF NOT EXISTS artists (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), icon varchar(255), UNIQUE(normalizedName))");
	createDb.exec("CREATE TABLE IF NOT EXISTS albums (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), " \
		"year INTEGER, cover varchar(255), artistId INTEGER, host varchar(255), icon varchar(255), UNIQUE(normalizedName, artistId))");
	QString createTableTracks = "CREATE TABLE IF NOT EXISTS tracks (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, " \
		"title varchar(255), artistId INTEGER, albumId INTEGER, artistAlbum varchar(255), length INTEGER, " \
		"rating INTEGER, disc INTEGER, internalCover INTEGER DEFAULT 0, host varchar(255), icon varchar(255))";
	createDb.exec(createTableTracks);
	transaction();

	// Foreach file, insert tuple
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
void SqlDatabase::saveCoverRef(const QString &coverPath, const QString &track)
{
	FileHelper fh(track);
	QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();
	QString artistNorm = this->normalizeField(artistAlbum);
	QString albumNorm = this->normalizeField(fh.album());

	uint artistId = qHash(artistNorm);
	uint albumId = qHash(albumNorm + QString::number(artistId));

	QSqlQuery updateCoverPath("UPDATE albums SET cover = ? WHERE id = ? AND artistId = ?", *this);
	updateCoverPath.addBindValue(coverPath);
	updateCoverPath.addBindValue(albumId);
	updateCoverPath.addBindValue(artistId);
	bool b = updateCoverPath.exec();
	if (b) {
		QSqlQuery selectAlbum("SELECT id, name, normalizedName, year, cover, artistId FROM albums WHERE id = ? AND artistId = ?", *this);
		selectAlbum.addBindValue(albumId);
		selectAlbum.addBindValue(artistId);
		if (selectAlbum.exec() && selectAlbum.next()) {
			if (AlbumDAO *albumDAO = qobject_cast<AlbumDAO*>(_cache.value(albumId))) {
				int i = -1;
				albumDAO->setId(selectAlbum.record().value(++i).toString());
				albumDAO->setTitle(selectAlbum.record().value(++i).toString());
				albumDAO->setTitleNormalized(selectAlbum.record().value(++i).toString());
				albumDAO->setYear(selectAlbum.record().value(++i).toString());
				albumDAO->setCover(selectAlbum.record().value(++i).toString());
				emit aboutToUpdateNode(albumDAO);
			}
		}
	}
}

QString SqlDatabase::normalizeField(const QString &s) const
{
	static QRegularExpression regExp("[^\\w]");
	return s.toLower().normalized(QString::NormalizationForm_KD).remove(regExp).trimmed();
}

/** Reads a file from the filesystem and adds it into the library. */
void SqlDatabase::saveFileRef(const QString &absFilePath)
{
	if (!isOpen()) {
		open();
	}
	FileHelper fh(absFilePath);
	if (fh.isValid()) {
		bool newArtistFound = false;
		bool newAlbumFound = false;
		ArtistDAO *artistDAO = NULL;
		AlbumDAO *albumDAO = NULL;

		QSqlQuery insertTrack(*this);

		insertTrack.prepare("INSERT INTO tracks (uri, trackNumber, title, artistId, albumId, artistAlbum, length, " \
			"disc, internalCover) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

		QString tn = fh.trackNumber();
		QString title = fh.title();
		QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();
		// Use Artist Album to reference tracks in table "tracks", not Artist
		QString artistNorm = this->normalizeField(artistAlbum);
		QString album = fh.album();
		QString albumNorm = this->normalizeField(album);
		QString length = fh.length();
		uint artistId = qHash(artistNorm);
		uint albumId = qHash(albumNorm + QString::number(artistId));
		int dn = fh.discNumber();

		insertTrack.addBindValue("file://" + absFilePath);
		insertTrack.addBindValue(tn.toInt());
		insertTrack.addBindValue(title);
		insertTrack.addBindValue(artistId);
		insertTrack.addBindValue(albumId);
		insertTrack.addBindValue(fh.artistAlbum());
		insertTrack.addBindValue(length);
		insertTrack.addBindValue(dn);
		insertTrack.addBindValue(fh.hasCover());

		TrackDAO *trackDAO = new TrackDAO;
		trackDAO->setUri("file://" + absFilePath);
		trackDAO->setTrackNumber(tn);
		trackDAO->setTitle(title);
		trackDAO->setArtist(fh.artist());
		trackDAO->setAlbum(album);
		trackDAO->setArtistAlbum(artistAlbum);
		trackDAO->setLength(length);
		trackDAO->setRating(fh.rating());
		trackDAO->setDisc(QString::number(dn));
		/// FIXME Internal Cover

		if (insertTrack.exec()) {
			QSqlQuery selectAlbum(*this);
			selectAlbum.prepare("SELECT name, normalizedName FROM albums WHERE id = ? AND artistId = ?");
			selectAlbum.addBindValue(albumId);
			selectAlbum.addBindValue(artistId);
			selectAlbum.exec();
			if (!selectAlbum.next()) {
				QSqlQuery insertAlbum(*this);
				insertAlbum.prepare("INSERT INTO albums (id, name, normalizedName, year, artistId) VALUES (?, ?, ?, ?, ?)");
				QString album = fh.album();
				insertAlbum.addBindValue(albumId);
				insertAlbum.addBindValue(album);
				insertAlbum.addBindValue(albumNorm);
				insertAlbum.addBindValue(fh.year());
				insertAlbum.addBindValue(artistId);
				insertAlbum.exec();
				newAlbumFound = true;

				albumDAO = new AlbumDAO;
				albumDAO->setId(QString::number(artistId));
				albumDAO->setTitle(album);
				albumDAO->setTitleNormalized(albumNorm);
				albumDAO->setYear(fh.year());
				if (fh.hasCover()) {
					albumDAO->setCover(absFilePath);
				}
				_cache.insert(albumId, albumDAO);
			} else {
				// A previous record exists for this normalized name but the new name is different
				if (QString::compare(selectAlbum.record().value(0).toString(), album) != 0) {
					QSqlQuery updateAlbum(*this);
					updateAlbum.prepare("UPDATE albums SET name = ? WHERE id = ?");
					updateAlbum.addBindValue(album);
					updateAlbum.addBindValue(albumId);
					updateAlbum.exec();
				}
			}

			QSqlQuery selectArtist(*this);
			selectArtist.prepare("SELECT name, normalizedName FROM artists WHERE id = ?");
			selectArtist.addBindValue(artistId);
			selectArtist.exec();
			if (!selectArtist.next()) {
				QSqlQuery insertArtist(*this);
				insertArtist.prepare("INSERT INTO artists (id, name, normalizedName) VALUES (?, ?, ?)");
				insertArtist.addBindValue(artistId);
				insertArtist.addBindValue(artistAlbum);
				insertArtist.addBindValue(artistNorm);
				insertArtist.exec();
				newArtistFound = true;

				artistDAO = new ArtistDAO;
				artistDAO->setId(QString::number(artistId));
				artistDAO->setTitle(artistAlbum);
				artistDAO->setTitleNormalized(artistNorm);
				_cache.insert(artistId, artistDAO);
			} else {
				if (QString::compare(selectArtist.record().value(0).toString(), artistAlbum) != 0) {
					QSqlQuery updateArtist(*this);
					updateArtist.prepare("UPDATE artists SET name = ? WHERE id = ?");
					updateArtist.addBindValue(artistAlbum);
					updateArtist.addBindValue(artistId);
					updateArtist.exec();
				}
			}
		}

		switch (SettingsPrivate::instance()->value("insertPolicy").toInt()) {
		case IP_Artists:
			if (newArtistFound) {
				emit nodeExtracted(artistDAO);
			}
			if (newAlbumFound) {
				if (artistDAO == NULL) {
					artistDAO = qobject_cast<ArtistDAO*>(_cache.value(artistId));
				}
				albumDAO->setParentNode(artistDAO);
				emit nodeExtracted(albumDAO);
			} else {
				albumDAO = qobject_cast<AlbumDAO*>(_cache.value(albumId));
			}
			trackDAO->setParentNode(albumDAO);
			emit nodeExtracted(trackDAO);
			break;
		case IP_Albums:
			break;
		case IP_ArtistsAlbums:
			break;
		case IP_Years:
			break;
		}
	} else {
		qDebug() << "Invalid file for:" << absFilePath;
	}
}
