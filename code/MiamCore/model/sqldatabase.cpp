#include "sqldatabase.h"

#include <QApplication>
#include <QDir>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>

#include <QtDebug>

#include "cover.h"
#include "settingsprivate.h"
#include "musicsearchengine.h"
#include "filehelper.h"
#include "yeardao.h"

SqlDatabase* SqlDatabase::_sqlDatabase = NULL;

SqlDatabase::SqlDatabase()
	: QObject(), QSqlDatabase("QSQLITE")
{
	_musicSearchEngine = new MusicSearchEngine;
	SettingsPrivate *settings = SettingsPrivate::instance();
	QString path("%1/%2/%3");
	path = path.arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation),
					settings->organizationName(),
					settings->applicationName());
	QString dbPath = QDir::toNativeSeparators(path + "/mp.db");
	QDir userDataPath(path);
	// Init a new database file for settings
	QFile dbFile(dbPath);
	if (!userDataPath.exists(path)) {
		// No DB path -> first launch
		if (!userDataPath.mkpath(path)) {
			qWarning() << tr("Cannot create path to store cache. Miam-Player might be able to run but it will be in limited mode");
		}
	} else if (!dbFile.exists()) {
		// DB folder exists but DB file doesn't. Did you delete DB file?
		// Wait for a few seconds and restart full scan
		/// TODO: full rescan <> rebuild which is only for local tracks
		/// Remote tracks (like Deezer) are still not synchronized
		QTimer *t = new QTimer(this);
		t->setSingleShot(true);
		t->start(5000);
		connect(t, &QTimer::timeout, this, static_cast<void (SqlDatabase::*)(void)>(&SqlDatabase::rebuild));
	}
	dbFile.open(QIODevice::ReadWrite);
	dbFile.close();
	setDatabaseName(dbPath);

	if (open()) {
		this->exec("PRAGMA journal_mode = MEMORY");
		this->exec("PRAGMA synchronous = OFF");
		this->exec("PRAGMA temp_store = 2");
		this->exec("PRAGMA foreign_keys = 1");
		QSqlQuery createDb(*this);
		createDb.exec("CREATE TABLE IF NOT EXISTS artists (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), icon varchar(255), UNIQUE(normalizedName))");
		createDb.exec("CREATE TABLE IF NOT EXISTS albums (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), " \
			"year INTEGER, cover varchar(255), artistId INTEGER, host varchar(255), icon varchar(255), UNIQUE(id, artistId))");
		QString createTableTracks = "CREATE TABLE IF NOT EXISTS tracks (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, " \
			"title varchar(255), artistId INTEGER, albumId INTEGER, artistAlbum varchar(255), length INTEGER, " \
			"rating INTEGER, disc INTEGER, internalCover INTEGER DEFAULT 0, host varchar(255), icon varchar(255))";
		createDb.exec(createTableTracks);
		createDb.exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, icon varchar(255), background varchar(255), checksum varchar(255))");
		createDb.exec("CREATE TABLE IF NOT EXISTS playlistTracks (trackNumber INTEGER, title varchar(255), album varchar(255), length INTEGER, " \
			 "artist varchar(255), rating INTEGER, year INTEGER, icon varchar(255), id INTEGER, url varchar(255), playlistId INTEGER, " \
			 "FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
	}

	connect(_musicSearchEngine, &MusicSearchEngine::progressChanged, this, &SqlDatabase::progressChanged);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedCover, this, &SqlDatabase::saveCoverRef);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedFile, this, &SqlDatabase::saveFileRef);

	// When the scan is complete, save the model in the filesystem
	connect(_musicSearchEngine, &MusicSearchEngine::searchHasEnded, [=] () {
		commit();
		QSqlQuery index(*this);
		index.exec("CREATE INDEX IF NOT EXISTS indexArtist ON tracks (artistId)");
		index.exec("CREATE INDEX IF NOT EXISTS indexAlbum ON tracks (albumId)");
		index.exec("CREATE INDEX IF NOT EXISTS indexPath ON tracks (uri)");
		index.exec("CREATE INDEX IF NOT EXISTS indexArtistId ON artists (id)");
		index.exec("CREATE INDEX IF NOT EXISTS indexAlbumId ON albums (id)");
		this->loadFromFileDB(false);
	});
}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
SqlDatabase* SqlDatabase::instance()
{
	if (_sqlDatabase == NULL) {
		_sqlDatabase = new SqlDatabase;
	}
	return _sqlDatabase;
}

bool SqlDatabase::insertIntoTableArtists(ArtistDAO *artist)
{
	QSqlQuery insertArtist(*this);
	insertArtist.prepare("INSERT OR IGNORE INTO artists (id, name, normalizedName) VALUES (?, ?, ?)");
	QString artistNorm = this->normalizeField(artist->title());
	uint artistId = qHash(artistNorm);

	insertArtist.addBindValue(artistId);
	insertArtist.addBindValue(artist->title());
	insertArtist.addBindValue(artistNorm);
	if (insertArtist.exec()) {
		emit nodeExtracted(artist);
	}

	return lastError().type() == QSqlError::NoError;
}

bool SqlDatabase::insertIntoTableAlbums(uint artistId, AlbumDAO *album)
{
	QSqlQuery insertAlbum(*this);
	insertAlbum.prepare("INSERT OR IGNORE INTO albums (id, name, normalizedName, year, artistId, host, icon) VALUES (?, ?, ?, ?, ?, ?, ?)");
	QString albumNorm = this->normalizeField(album->title());
	uint albumId = artistId + qHash(albumNorm, 1);

	insertAlbum.addBindValue(albumId);
	insertAlbum.addBindValue(album->title());
	insertAlbum.addBindValue(albumNorm);
	insertAlbum.addBindValue(album->year());
	insertAlbum.addBindValue(artistId);
	insertAlbum.addBindValue(album->host());
	insertAlbum.addBindValue(album->icon());
	if (insertAlbum.exec()) {
		if (ArtistDAO *artist = qobject_cast<ArtistDAO*>(_cache.value(artistId))) {
			album->setParentNode(artist);
			emit nodeExtracted(album);
		} else {
			qDebug() << Q_FUNC_INFO << "artist wasn't found?" << artistId;
		}
	}

	return lastError().type() == QSqlError::NoError;
}

int SqlDatabase::insertIntoTablePlaylists(const PlaylistDAO &playlist, const std::list<TrackDAO> &tracks, bool isOverwriting)
{
	int id;
	if (isOverwriting) {
		id = playlist.id().toInt();
		QSqlQuery update(*this);
		update.prepare("UPDATE playlists SET title = ?, checksum = ? WHERE id = ?");
		update.addBindValue(playlist.title());
		update.addBindValue(playlist.checksum());
		update.addBindValue(id);
		update.exec();
		this->insertIntoTablePlaylistTracks(id, tracks, isOverwriting);
	} else {
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
		qDebug() << Q_FUNC_INFO << "inserting new playlist" << playlist.checksum();

		bool b = insert.exec();
		int i = 0;
		while (!b && i < 10) {
			insert.bindValue(":id", QString::number(qrand()));
			b = insert.exec();
			++i;
			qDebug() << Q_FUNC_INFO << "insert failed for playlist" << playlist.title() << "trying again" << i;
			qDebug() << Q_FUNC_INFO << insert.lastError();
		}
		this->insertIntoTablePlaylistTracks(id, tracks);
	}
	return id;
}

bool SqlDatabase::insertIntoTablePlaylistTracks(int playlistId, const std::list<TrackDAO> &tracks, bool isOverwriting)
{
	this->transaction();
	if (isOverwriting) {
		QSqlQuery deleteTracks(*this);
		deleteTracks.prepare("DELETE FROM playlistTracks WHERE playlistId = ?");
		deleteTracks.addBindValue(playlistId);
		deleteTracks.exec();
	}
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
	return lastError().type() == QSqlError::NoError;
}

bool SqlDatabase::insertIntoTableTracks(const TrackDAO &track)
{
	QSqlQuery insertTrack(*this);
	insertTrack.prepare("INSERT INTO tracks (uri, trackNumber, title, artistId, albumId, artistAlbum, length, rating, " \
		"disc, host, icon) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	QString artistAlbum = track.artistAlbum().isEmpty() ? track.artist() : track.artistAlbum();
	QString artistNorm = this->normalizeField(artistAlbum);
	QString albumNorm = this->normalizeField(track.album());
	uint artistId = qHash(artistNorm);
	uint albumId = artistId + qHash(albumNorm, 1);

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
	//close();
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

bool SqlDatabase::removePlaylists(const QList<PlaylistDAO> &playlists)
{
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
	return this->commit();
}

Cover* SqlDatabase::selectCoverFromURI(const QString &uri)
{
	Cover *c = NULL;

	QSqlQuery selectCover(*this);
	selectCover.prepare("SELECT t.internalCover, a.cover, a.id FROM albums a INNER JOIN tracks t ON a.id = t.albumId " \
		"WHERE t.uri = ?");
	selectCover.addBindValue(uri);
	if (selectCover.exec() && selectCover.next()) {
		bool internalCover = selectCover.record().value(0).toBool();
		QString coverPath = selectCover.record().value(1).toString();
		uint albumId = selectCover.record().value(2).toUInt();
		if (internalCover || !coverPath.isEmpty()) {
			// If URI has an internal cover, i.e. uri points to a local file
			if (internalCover) {
				FileHelper fh(uri);
				c = fh.extractCover();
			} else {
				c = new Cover(coverPath);
			}
		} else {
			// No direct cover for this file, let's search for the entire album if one track has an inner cover
			selectCover.prepare("SELECT uri FROM tracks WHERE albumId = ? AND internalCover = 1 LIMIT 1");
			selectCover.addBindValue(albumId);
			if (selectCover.exec() && selectCover.next()) {
				FileHelper fh(selectCover.record().value(0).toString());
				c = fh.extractCover();
			}
		}
	}
	return c;
}

QList<TrackDAO> SqlDatabase::selectPlaylistTracks(int playlistID)
{
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
			tracks.append(std::move(track));
		}
	}
	return tracks;
}

PlaylistDAO SqlDatabase::selectPlaylist(int playlistId)
{
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
	return playlist;
}

QList<PlaylistDAO> SqlDatabase::selectPlaylists()
{
	QList<PlaylistDAO> playlists;
	QSqlQuery results = exec("SELECT title, id, icon, background, checksum FROM playlists");
	while (results.next()) {
		PlaylistDAO playlist;
		int i = -1;
		playlist.setTitle(results.record().value(++i).toString());
		playlist.setId(results.record().value(++i).toString());
		playlist.setIcon(results.record().value(++i).toString());
		playlist.setBackground(results.record().value(++i).toString());
		playlist.setChecksum(results.record().value(++i).toString());
		playlists.append(std::move(playlist));
	}

	return playlists;
}

TrackDAO SqlDatabase::selectTrack(const QString &uri)
{
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
	return track;
}

bool SqlDatabase::playlistHasBackgroundImage(int playlistID)
{
	QSqlQuery query = exec("SELECT background FROM playlists WHERE id = " + QString::number(playlistID));
	query.next();
	bool result = !query.record().value(0).toString().isEmpty();
	qDebug() << Q_FUNC_INFO << query.record().value(0).toString() << result;
	return result;
}

void SqlDatabase::updateTablePlaylistWithBackgroundImage(int playlistID, const QString &backgroundImagePath)
{
	QSqlQuery update(*this);
	update.prepare("UPDATE playlists SET background = ? WHERE id = ?");
	update.addBindValue(backgroundImagePath);
	update.addBindValue(playlistID);
	update.exec();
}

void SqlDatabase::updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist)
{
	QSqlQuery update(*this);
	update.prepare("UPDATE albums SET cover = ? WHERE normalizedName = ? AND artistId = (SELECT id FROM artists WHERE normalizedName = ?)");
	update.addBindValue(coverPath);
	update.addBindValue(this->normalizeField(album));
	update.addBindValue(this->normalizeField(artist));
	update.exec();
}

/**
 * Update a list of tracks. If track name has changed, will be removed from Library then added right after.
 * \param tracksToUpdate 'First' in pair is actual filename, 'Second' is the new filename, but may be empty.*/
void SqlDatabase::updateTracks(const QList<QPair<QString, QString>> &tracksToUpdate)
{
	// Signals are blocked to prevent saveFileRef method to emit one. Load method will tell connected views to rebuild themselves
	this->blockSignals(true);
	transaction();

	/// TODO: First, select old artistId and albumId
	/// Update artistId, albumId if modified

	for (int i = 0; i < tracksToUpdate.length(); i++) {
		QPair<QString, QString> pair = tracksToUpdate.at(i);
		// Old path, New Path. If New Path exists, then fileName has changed.
		if (pair.second.isEmpty()) {
			FileHelper fh(pair.first);

			QSqlQuery selectArtist(*this);
			selectArtist.prepare("SELECT artistId, albumId FROM tracks WHERE uri = ?");
			selectArtist.addBindValue(pair.first);
			uint oldArtistId = 0;
			uint oldAlbumId = 0;
			if (selectArtist.exec() && selectArtist.next()) {
				oldArtistId = selectArtist.record().value(0).toUInt();
				oldAlbumId = selectArtist.record().value(1).toUInt();
			}

			QSqlQuery updateTrack(*this);
			updateTrack.prepare("UPDATE tracks SET album = ?, albumId = ?, artist = ?, artistId = ?, artistAlbum = ?, disc = ?, internalCover = ?, title = ?, trackNumber = ? WHERE uri = ?");

			QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();
			QString artistNorm = this->normalizeField(artistAlbum);
			QString albumNorm = this->normalizeField(fh.album());
			uint artistId = qHash(artistNorm);
			uint albumId = artistId + qHash(albumNorm, 1);

			updateTrack.addBindValue(fh.album());
			updateTrack.addBindValue(albumId);
			updateTrack.addBindValue(fh.artist());
			updateTrack.addBindValue(artistId);
			updateTrack.addBindValue(fh.artistAlbum());
			updateTrack.addBindValue(fh.discNumber());
			updateTrack.addBindValue(fh.hasCover());
			updateTrack.addBindValue(fh.title());
			updateTrack.addBindValue(fh.trackNumber().toInt());
			updateTrack.addBindValue(QDir::toNativeSeparators(pair.first));
			updateTrack.exec();

			QSqlQuery updateAlbum(*this);
			updateAlbum.prepare("UPDATE albums SET year = ? WHERE id = ?");
			updateAlbum.addBindValue(fh.year().toInt());
			updateAlbum.addBindValue(albumId);
			updateAlbum.exec();

			// Check if old album has no more tracks
			if (oldAlbumId != albumId) {
				/// TODO
			}
		} else {
			QSqlQuery hasTrack("SELECT COUNT(*) FROM tracks WHERE uri = ?", *this);
			hasTrack.addBindValue(QDir::toNativeSeparators(pair.first));
			if (hasTrack.exec() && hasTrack.next() && hasTrack.record().value(0).toInt() != 0) {
				QSqlQuery removeTrack("DELETE FROM tracks WHERE uri = ?", *this);
				removeTrack.addBindValue(QDir::toNativeSeparators(pair.first));
				qDebug() << Q_FUNC_INFO << "deleting tracks";
				if (removeTrack.exec()) {
					this->saveFileRef(pair.second);
				}
			}
		}
	}
	commit();
	this->blockSignals(false);

	/// XXX: might not be the smartest way to reload changes, but it's way simpler than searching in a tree for modifications
	this->load();
}

/** Read all tracks entries in the database and send them to connected views. */
void SqlDatabase::loadFromFileDB(bool sendResetSignal)
{
	qDebug() << Q_FUNC_INFO << sendResetSignal;
	if (sendResetSignal) {
		emit aboutToLoad();
	}

	// Lambda function to reduce duplicate code which is relevant only in this method
	auto loadTracks = [this] (QSqlQuery& qTracks, AlbumDAO *albumDAO, const QString &year) -> void {
		bool internalCover = false;
		while (qTracks.next()) {
			QSqlRecord r = qTracks.record();
			TrackDAO *trackDAO = new TrackDAO;
			QString uri = r.value(0).toString();
			trackDAO->setUri(uri);
			trackDAO->setTrackNumber(r.value(1).toString());
			trackDAO->setTitle(r.value(2).toString());
			trackDAO->setArtist(r.value(3).toString());
			trackDAO->setAlbum(r.value(4).toString());
			trackDAO->setArtistAlbum(r.value(5).toString());
			trackDAO->setLength(r.value(6).toString());
			trackDAO->setRating(r.value(7).toInt());
			trackDAO->setDisc(r.value(8).toString());
			if (!internalCover && r.value(9).toBool()) {
				albumDAO->setCover(uri);
				internalCover = true;
			}
			trackDAO->setHost(r.value(10).toString());
			trackDAO->setIcon(r.value(11).toString());
			trackDAO->setParentNode(albumDAO);
			trackDAO->setYear(year);
			emit nodeExtracted(trackDAO);
		}
		if (internalCover) {
			emit aboutToUpdateNode(albumDAO);
		}
	};

	auto s = SettingsPrivate::instance();
	switch (s->insertPolicy()) {
	case SettingsPrivate::IP_Artists: {
		QStringList filters;
		if (s->isLibraryFilteredByArticles() && !s->libraryFilteredByArticles().isEmpty()) {
			filters = s->libraryFilteredByArticles();
		}

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
				if (filters.isEmpty()) {
					artistDAO->setTitleNormalized(record.value(2).toString());
				} else {
					for (QString filter : filters) {
						if (artist.startsWith(filter + " ", Qt::CaseInsensitive)) {
							artist = artist.mid(filter.length() + 1);
							artistDAO->setCustomData(artist + ", " + filter);
							break;
						}
					}
					artistDAO->setTitleNormalized(this->normalizeField(artist));
				}
				emit nodeExtracted(artistDAO);
				//_cache.insert(artistId, artistDAO);

				// Level 2: Albums
				QSqlQuery qAlbums(*this);
				qAlbums.prepare("SELECT name, normalizedName, year, cover, host, icon, id FROM albums WHERE artistId = ?");
				qAlbums.addBindValue(artistId);
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO *albumDAO = new AlbumDAO;
						QString album = r.value(0).toString();
						albumDAO->setTitle(album);
						albumDAO->setTitleNormalized(r.value(1).toString());
						QString year = r.value(2).toString();
						albumDAO->setYear(year);
						albumDAO->setCover(r.value(3).toString());
						albumDAO->setHost(r.value(4).toString());
						albumDAO->setIcon(r.value(5).toString());
						uint albumId = r.value(6).toUInt();
						albumDAO->setParentNode(artistDAO);
						emit nodeExtracted(albumDAO);
						//_cache.insert(albumId, albumDAO);

						// Level 3: Tracks
						QSqlQuery qTracks(*this);
						qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
										"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
										"INNER JOIN artists art ON t.artistId = art.id " \
										"WHERE t.artistId = ? AND t.albumId = ?");
						qTracks.addBindValue(artistId);
						qTracks.addBindValue(albumId);
						if (qTracks.exec()) {
							loadTracks(qTracks, albumDAO, year);
						}
					}
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_Albums: {
		// Level 1: Albums
		QSqlQuery qAlbums("SELECT name, normalizedName, year, cover, host, icon, id FROM albums", *this);
		if (qAlbums.exec()) {
			while (qAlbums.next()) {
				QSqlRecord r = qAlbums.record();
				AlbumDAO *albumDAO = new AlbumDAO;
				QString album = r.value(0).toString();
				albumDAO->setTitle(album);
				albumDAO->setTitleNormalized(r.value(1).toString());
				QString year = r.value(2).toString();
				albumDAO->setYear(year);
				albumDAO->setCover(r.value(3).toString());
				albumDAO->setHost(r.value(4).toString());
				albumDAO->setIcon(r.value(5).toString());
				uint albumId = r.value(6).toUInt();
				emit nodeExtracted(albumDAO);
				//_cache.insert(albumId, albumDAO);

				// Level 2: Tracks
				QSqlQuery qTracks(*this);
				qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
								"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
								"INNER JOIN artists art ON t.artistId = art.id " \
								"WHERE t.albumId = ?");
				qTracks.addBindValue(albumId);
				if (qTracks.exec()) {
					loadTracks(qTracks, albumDAO, year);
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_ArtistsAlbums: {
		// Level 1: Artist - Album
		QSqlQuery qAlbums("SELECT art.name || ' – ' || alb.name, art.normalizedName || alb.normalizedName, alb.year, alb.cover, alb.host, alb.icon, alb.id " \
						  "FROM albums alb " \
						  "INNER JOIN artists art ON alb.artistId = art.id", *this);
		if (qAlbums.exec()) {
			while (qAlbums.next()) {
				QSqlRecord r = qAlbums.record();
				AlbumDAO *albumDAO = new AlbumDAO;
				QString album = r.value(0).toString();
				albumDAO->setTitle(album);
				albumDAO->setTitleNormalized(r.value(1).toString());
				QString year = r.value(2).toString();
				albumDAO->setYear(year);
				albumDAO->setCover(r.value(3).toString());
				albumDAO->setHost(r.value(4).toString());
				albumDAO->setIcon(r.value(5).toString());
				uint albumId = r.value(6).toUInt();
				emit nodeExtracted(albumDAO);
				//_cache.insert(albumId, albumDAO);

				// Level 2: Tracks
				QSqlQuery qTracks(*this);
				qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
								"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
								"INNER JOIN artists art ON t.artistId = art.id " \
								"WHERE t.albumId = ?");
				qTracks.addBindValue(albumId);
				if (qTracks.exec()) {
					loadTracks(qTracks, albumDAO, year);
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_Years: {
		// Level 1: Years
		QSqlQuery qYears("SELECT DISTINCT year FROM albums ORDER BY year", *this);
		if (qYears.exec()) {
			while (qYears.next()) {
				QSqlRecord r = qYears.record();
				YearDAO *yearDAO = new YearDAO;
				QVariant vYear = r.value(0);
				yearDAO->setTitle(vYear.toString());
				yearDAO->setTitleNormalized(vYear.toString());
				emit nodeExtracted(yearDAO);

				// Level 2: Artist - Album
				QSqlQuery qAlbums(*this);
				qAlbums.prepare("SELECT art.name || ' – ' || alb.name, art.normalizedName || alb.normalizedName, alb.year, alb.cover, alb.host, alb.icon, art.id, alb.id " \
								"FROM albums alb INNER JOIN artists art ON alb.artistId = art.id " \
								"WHERE alb.year = ?");
				qAlbums.addBindValue(vYear.toInt());
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO *albumDAO = new AlbumDAO;
						QString album = r.value(0).toString();
						albumDAO->setTitle(album);
						albumDAO->setTitleNormalized(r.value(1).toString());
						QString year = r.value(2).toString();
						albumDAO->setYear(year);
						albumDAO->setCover(r.value(3).toString());
						albumDAO->setHost(r.value(4).toString());
						albumDAO->setIcon(r.value(5).toString());
						uint artistId = r.value(6).toUInt();
						uint albumId = r.value(7).toUInt();
						albumDAO->setParentNode(yearDAO);
						emit nodeExtracted(albumDAO);
						//_cache.insert(albumId, albumDAO);

						// Level 3: Tracks
						QSqlQuery qTracks(*this);
						qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
										"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
										"INNER JOIN artists art ON t.artistId = art.id " \
										"WHERE t.artistId = ? AND t.albumId = ?");
						qTracks.addBindValue(artistId);
						qTracks.addBindValue(albumId);
						if (qTracks.exec()) {
							loadTracks(qTracks, albumDAO, year);
						}
					}
				}
			}
		}
		break;
	}
	}

	emit loaded();
}

/** Delete and rescan local tracks. */
void SqlDatabase::rebuild()
{
	emit aboutToLoad();

	open();
	this->exec("PRAGMA journal_mode = MEMORY");
	this->exec("PRAGMA synchronous = OFF");
	this->exec("PRAGMA temp_store = 2");
	this->exec("PRAGMA foreign_keys = 1");

	QSqlQuery createDb(*this);
	createDb.exec("DELETE FROM tracks WHERE uri LIKE 'file:%'");
	createDb.exec("DELETE FROM albums WHERE id NOT IN (SELECT DISTINCT albumId FROM tracks)");
	createDb.exec("DELETE FROM artists WHERE id NOT IN (SELECT DISTINCT artistId FROM tracks)");
	createDb.exec("DROP INDEX indexArtist");
	createDb.exec("DROP INDEX indexAlbum");
	createDb.exec("DROP INDEX indexPath");
	createDb.exec("DROP INDEX indexArtistId");
	createDb.exec("DROP INDEX indexAlbumId");
	transaction();

	// Foreach file, insert tuple
	_musicSearchEngine->moveToThread(&_workerThread);
	_musicSearchEngine->doSearch();
}

void SqlDatabase::rebuild(const QStringList &oldLocations, const QStringList &newLocations)
{
	open();
	this->exec("PRAGMA journal_mode = MEMORY");
	this->exec("PRAGMA synchronous = OFF");
	this->exec("PRAGMA temp_store = 2");
	this->exec("PRAGMA foreign_keys = 1");

	// Remove old locations from database cache
	transaction();
	foreach (QString oldLocation, oldLocations) {
		if (!newLocations.contains(oldLocation)) {
			QSqlQuery syncDb(*this);
			syncDb.prepare("DELETE FROM tracks WHERE uri LIKE :path ");
			syncDb.bindValue(":path", "file://" + QDir::fromNativeSeparators(oldLocation) + "%");
			syncDb.exec();
			syncDb.exec("DELETE FROM albums WHERE id NOT IN (SELECT DISTINCT albumId FROM tracks)");
			syncDb.exec("DELETE FROM artists WHERE id NOT IN (SELECT DISTINCT artistId FROM tracks)");
		}
	}
	commit();

	loadFromFileDB();

	// Restart the worker thread on new locations
	QStringList locationsToAdd;
	foreach (QString newLocation, newLocations) {
		if (!oldLocations.contains(newLocation)) {
			locationsToAdd.append(newLocation);
		}
	}

	if (!locationsToAdd.isEmpty()) {
		_musicSearchEngine->moveToThread(&_workerThread);
		_musicSearchEngine->doSearch(locationsToAdd);
	}
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
	uint albumId = artistId + qHash(albumNorm, 1);

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
	QString sNormed = s.toLower().normalized(QString::NormalizationForm_KD).remove(regExp).trimmed();
	if (sNormed.isEmpty()) {
		return s.toLower().remove(" ").trimmed();
	} else {
		return sNormed;
	}
}

/** Reads a file from the filesystem and adds it into the library. */
void SqlDatabase::saveFileRef(const QString &absFilePath)
{
	if (!isOpen()) {
		open();
	}
	FileHelper fh(absFilePath);
	if (!fh.isValid()) {
		return;
	}

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
	uint albumId = artistId + qHash(albumNorm, 1);
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

	bool artistInserted = false;
	bool albumInserted = false;
	ArtistDAO *artistDAO = NULL;
	AlbumDAO *albumDAO = NULL;

	if (!insertTrack.exec()) {
		qDebug() << Q_FUNC_INFO << insertTrack.lastError();
		return;
	}
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

	QSqlQuery selectAlbum(*this);
	selectAlbum.prepare("SELECT name, normalizedName FROM albums WHERE id = ? AND artistId = ?");
	selectAlbum.addBindValue(albumId);
	selectAlbum.addBindValue(artistId);
	selectAlbum.exec();
	if (!selectAlbum.next()) {
		QSqlQuery insertAlbum(*this);
		insertAlbum.prepare("INSERT INTO albums (id, name, normalizedName, year, artistId) VALUES (?, ?, ?, ?, ?)");
		insertAlbum.addBindValue(albumId);
		insertAlbum.addBindValue(album);
		insertAlbum.addBindValue(albumNorm);
		if (fh.year().isEmpty()) {
			insertAlbum.addBindValue(0);
		} else {
			insertAlbum.addBindValue(fh.year());
		}
		insertAlbum.addBindValue(artistId);
		albumInserted = insertAlbum.exec();
		if (!albumInserted) {
			qDebug() << Q_FUNC_INFO << "not inserted" << insertAlbum.lastError();
		}

		albumDAO = new AlbumDAO;
		albumDAO->setId(QString::number(albumId));
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
			qDebug() << Q_FUNC_INFO << "updating" << album << albumNorm << albumId;
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
		artistInserted = insertArtist.exec();

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
