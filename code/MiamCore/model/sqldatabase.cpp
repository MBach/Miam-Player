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

#include <chrono>
#include <random>

SqlDatabase* SqlDatabase::_sqlDatabase = nullptr;

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
		createDb.exec("CREATE TABLE IF NOT EXISTS artists (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), "\
					  " icon varchar(255), host varchar(255), UNIQUE(normalizedName))");
		createDb.exec("CREATE TABLE IF NOT EXISTS albums (id INTEGER PRIMARY KEY, name varchar(255), normalizedName varchar(255), " \
			"year INTEGER, cover varchar(255), artistId INTEGER, host varchar(255), icon varchar(255), UNIQUE(id, artistId))");
		QString createTableTracks = "CREATE TABLE IF NOT EXISTS tracks (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, " \
			"title varchar(255), artistId INTEGER, albumId INTEGER, artistAlbum varchar(255), length INTEGER, " \
			"rating INTEGER, disc INTEGER, internalCover INTEGER DEFAULT 0, host varchar(255), icon varchar(255))";
		createDb.exec(createTableTracks);
		createDb.exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, icon varchar(255), " \
					  "host varchar(255), background varchar(255), checksum varchar(255))");
		createDb.exec("CREATE TABLE IF NOT EXISTS playlistTracks (trackNumber INTEGER, title varchar(255), album varchar(255), length INTEGER, " \
					  "artist varchar(255), rating INTEGER, year INTEGER, icon varchar(255), host varchar(255), id INTEGER, " \
					  "url varchar(255), playlistId INTEGER, FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		/// TEST Monitor Filesystem
		 createDb.exec("CREATE TABLE IF NOT EXISTS filesystem (path VARCHAR(255) PRIMARY KEY ASC, " \
			"lastModified INTEGER);");
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

		qDebug() << Q_FUNC_INFO;
		this->loadFromFileDB(true);

		// Resync remote players and remote databases
		emit aboutToResyncRemoteSources();
	});
}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
SqlDatabase* SqlDatabase::instance()
{
	if (_sqlDatabase == nullptr) {
		_sqlDatabase = new SqlDatabase;
	}
	return _sqlDatabase;
}

MusicSearchEngine * SqlDatabase::musicSearchEngine() const
{
	return _musicSearchEngine;
}

bool SqlDatabase::insertIntoTableArtists(ArtistDAO *artist)
{
	QSqlQuery insertArtist(*this);
	insertArtist.prepare("INSERT OR IGNORE INTO artists (id, name, normalizedName, host) VALUES (?, ?, ?, ?)");
	QString artistNorm = this->normalizeField(artist->title());
	uint artistId = qHash(artistNorm);

	insertArtist.addBindValue(artistId);
	insertArtist.addBindValue(artist->title());
	insertArtist.addBindValue(artistNorm);
	insertArtist.addBindValue(artist->host());
	insertArtist.exec();

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
		if (ArtistDAO *artist = this->selectArtist(artistId)) {
			album->setParentNode(artist);
		}
	}

	return lastError().type() == QSqlError::NoError;
}

uint SqlDatabase::insertIntoTablePlaylists(const PlaylistDAO &playlist, const std::list<TrackDAO> &tracks, bool isOverwriting)
{
	static std::uniform_int_distribution<uint> tt;
	this->transaction();
	uint id;
	if (isOverwriting) {
		if (this->updateTablePlaylist(playlist)) {
			id = playlist.id().toUInt();
			this->insertIntoTablePlaylistTracks(id, tracks, isOverwriting);
		}
	} else {
		if (playlist.id().isEmpty()) {
			auto seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::mt19937_64 generator(seed);
			id = tt(generator);
		} else {
			id = playlist.id().toUInt();
		}

		QSqlQuery insert(*this);
		insert.prepare("INSERT INTO playlists(id, title, duration, icon, host, checksum) VALUES (?, ?, ?, ?, ?, ?)");
		insert.addBindValue(id);
		insert.addBindValue(playlist.title());
		insert.addBindValue(playlist.length());
		insert.addBindValue(playlist.icon());
		insert.addBindValue(playlist.host());
		insert.addBindValue(playlist.checksum());
		if (insert.exec()) {
			this->insertIntoTablePlaylistTracks(id, tracks);
		}
	}
	this->commit();
	return id;
}

bool SqlDatabase::insertIntoTablePlaylistTracks(uint playlistId, const std::list<TrackDAO> &tracks, bool isOverwriting)
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
		insert.prepare("INSERT INTO playlistTracks (trackNumber, title, album, length, artist, rating, year, " \
					   "icon, host, id, url, playlistId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		insert.addBindValue(track.trackNumber());
		insert.addBindValue(track.title());
		insert.addBindValue(track.album());
		insert.addBindValue(track.length());
		insert.addBindValue(track.artist());
		insert.addBindValue(track.rating());
		insert.addBindValue(track.year());
		insert.addBindValue(track.icon());
		insert.addBindValue(track.host());
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

bool SqlDatabase::removePlaylist(uint playlistId)
{
	this->transaction();
	/// XXX: CASCADE not working?
	QSqlQuery children(*this);
	children.prepare("DELETE FROM playlistTracks WHERE playlistId = :id");
	children.bindValue(":id", playlistId);
	children.exec();

	QSqlQuery remove(*this);
	remove.prepare("DELETE FROM playlists WHERE id = :id");
	remove.bindValue(":id", playlistId);
	remove.exec();
	return this->commit();
}

void SqlDatabase::removePlaylistsFromHost(const QString &host)
{
	this->transaction();

	QSqlQuery children(*this);
	children.prepare("DELETE FROM playlistTracks WHERE playlistId IN (SELECT id FROM playlists WHERE host LIKE :h)");
	children.bindValue(":h", host);
	children.exec();

	QSqlQuery remove(*this);
	remove.prepare("DELETE FROM playlists WHERE host LIKE :h");
	remove.bindValue(":h", host);
	remove.exec();

	this->commit();
}

void SqlDatabase::removeRecordsFromHost(const QString &host)
{
	qDebug() << Q_FUNC_INFO << host;
	this->transaction();
	QSqlQuery removeTracks(*this);
	removeTracks.prepare("DELETE FROM tracks WHERE host LIKE :h");
	removeTracks.bindValue(":h", host);
	removeTracks.exec();

	QSqlQuery removeAlbums(*this);
	removeAlbums.prepare("DELETE FROM albums WHERE host LIKE :h");
	removeAlbums.bindValue(":h", host);
	removeAlbums.exec();

	QSqlQuery removeArtists(*this);
	removeArtists.prepare("DELETE FROM artists WHERE host LIKE :h");
	removeArtists.bindValue(":h", host);
	removeArtists.exec();

	this->commit();
	qDebug() << Q_FUNC_INFO;
	this->loadFromFileDB();
}

Cover* SqlDatabase::selectCoverFromURI(const QString &uri)
{
	Cover *c = nullptr;

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

QList<TrackDAO> SqlDatabase::selectPlaylistTracks(uint playlistID)
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

PlaylistDAO SqlDatabase::selectPlaylist(uint playlistId)
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

ArtistDAO* SqlDatabase::selectArtist(uint artistId)
{
	QSqlQuery selectArtist(*this);
	selectArtist.prepare("SELECT id, name, normalizedName, icon, host FROM artists WHERE id = ?");
	selectArtist.addBindValue(artistId);
	if (selectArtist.exec() && selectArtist.next()) {
		ArtistDAO *artist = new ArtistDAO;
		int i = -1;
		artist->setId(selectArtist.record().value(++i).toString());
		artist->setTitle(selectArtist.record().value(++i).toString());
		artist->setTitleNormalized(selectArtist.record().value(++i).toString());
		artist->setIcon(selectArtist.record().value(++i).toString());
		artist->setHost(selectArtist.record().value(++i).toString());
		return artist;
	} else {
		return nullptr;
	}
}

TrackDAO SqlDatabase::selectTrackByURI(const QString &uri)
{
	TrackDAO track;
	QSqlQuery qTracks(*this);
	qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, " \
					"rating, disc, internalCover, t.host, t.icon, alb.year " \
					"FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
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

bool SqlDatabase::playlistHasBackgroundImage(uint playlistID)
{
	QSqlQuery query = exec("SELECT background FROM playlists WHERE id = " + QString::number(playlistID));
	query.next();
	bool result = !query.record().value(0).toString().isEmpty();
	qDebug() << Q_FUNC_INFO << query.record().value(0).toString() << result;
	return result;
}

bool SqlDatabase::updateTablePlaylist(const PlaylistDAO &playlist)
{
	QSqlQuery update(*this);
	update.prepare("UPDATE playlists SET title = ?, checksum = ? WHERE id = ?");
	update.addBindValue(playlist.title());
	update.addBindValue(playlist.checksum());
	update.addBindValue(playlist.id());
	return update.exec();
}

void SqlDatabase::updateTablePlaylistWithBackgroundImage(uint playlistID, const QString &backgroundImagePath)
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

/** Update a list of tracks. If track name has changed, will be removed from Library then added right after. */
void SqlDatabase::updateTracks(const QStringList &oldPaths, const QStringList &newPaths)
{
	// Signals are blocked to prevent saveFileRef method to emit one. Load method will tell connected views to rebuild themselves
	transaction();
	Q_ASSERT(oldPaths.size() == newPaths.size());

	QList<FileHelper*> olds;
	QList<FileHelper*> news;
	QList<ArtistDAO*> artists;
	QList<AlbumDAO*> albums;

	// If New Path exists, then fileName has changed.
	for (int i = 0; i < oldPaths.length(); i++) {
		QString oldPath = "file://" + oldPaths.at(i);
		if (newPaths.at(i).isEmpty()) {
			//qDebug() << oldPath;
			FileHelper *fh = new FileHelper(oldPath);

			QSqlQuery selectArtist(*this);
			selectArtist.prepare("SELECT artistId, albumId FROM tracks WHERE uri = ?");
			selectArtist.addBindValue(oldPath);
			uint oldArtistId = 0;
			uint oldAlbumId = 0;
			if (selectArtist.exec() && selectArtist.next()) {
				oldArtistId = selectArtist.record().value(0).toUInt();
				oldAlbumId = selectArtist.record().value(1).toUInt();
			}

			QSqlQuery updateTrack(*this);
			updateTrack.prepare("UPDATE tracks SET trackNumber = ?, title = ?, artistId = ?, albumId = ?, artistAlbum = ?, rating = ?, "\
								"disc = ?, internalCover = ? WHERE uri = ?");

			QString artistAlbum = fh->artistAlbum().isEmpty() ? fh->artist() : fh->artistAlbum();
			QString artistNorm = this->normalizeField(artistAlbum);
			QString albumNorm = this->normalizeField(fh->album());
			uint artistId = qHash(artistNorm);
			uint albumId = artistId + qHash(albumNorm, 1);

			// Check if Artist has changed (can change how tracks are displayed in the library)
			if (oldArtistId != artistId) {
				ArtistDAO *artistDAO = new ArtistDAO;
				artistDAO->setId(QString::number(artistId));
				artistDAO->setTitle(artistAlbum);
				artistDAO->setTitleNormalized(artistNorm);
				if (this->insertIntoTableArtists(artistDAO)) {
					emit nodeExtracted(artistDAO);
					artists << artistDAO;
				} else {
					delete artistDAO;
				}
			}

			// Same thing for Album
			if (oldAlbumId == albumId) {
				QSqlQuery queryAlbum("SELECT cover FROM albums WHERE id = ?", *this);
				queryAlbum.addBindValue(oldAlbumId);
				if (queryAlbum.exec() && queryAlbum.next()) {
					AlbumDAO *albumDAO = new AlbumDAO;
					albumDAO->setId(QString::number(oldAlbumId));
					albumDAO->setTitle(fh->album());
					albumDAO->setYear(fh->year());
					if (fh->hasCover()) {
						albumDAO->setCover(oldPath);
					} else {
						albumDAO->setCover(queryAlbum.record().value(0).toString());
					}
					albums << albumDAO;
					qDebug() << Q_FUNC_INFO << "saving temp albumDAO";
				}
			} else {
				QSqlQuery queryAlbum("SELECT * FROM albums WHERE id = ?", *this);
				queryAlbum.addBindValue(albumId);
				if (!(queryAlbum.exec() && queryAlbum.next())) {
					AlbumDAO *albumDAO = new AlbumDAO;
					albumDAO->setId(QString::number(albumId));
					albumDAO->setTitle(fh->album());
					albumDAO->setYear(fh->year());
					if (this->insertIntoTableAlbums(artistId, albumDAO)) {
						albums << albumDAO;
						if (fh->hasCover()) {
							albumDAO->setCover(oldPath);
						} else {
							// how to tie cover on the filesystem?
						}
						emit nodeExtracted(albumDAO);
					} else {
						delete albumDAO;
					}
				}
			}

			updateTrack.addBindValue(fh->trackNumber());
			updateTrack.addBindValue(fh->title());
			updateTrack.addBindValue(artistId);
			updateTrack.addBindValue(albumId);
			updateTrack.addBindValue(fh->artistAlbum());
			int rating = fh->rating();
			int discNumber = fh->discNumber();
			updateTrack.addBindValue(rating);
			updateTrack.addBindValue(discNumber);
			updateTrack.addBindValue(fh->hasCover());
			//qDebug() << Q_FUNC_INFO << oldPath << QDir::fromNativeSeparators(oldPath);
			updateTrack.addBindValue(oldPath);

			AlbumDAO *albumDAO = nullptr;
			for (AlbumDAO *savedAlbum : albums) {
				if (savedAlbum->id().toUInt() == albumId) {
					albumDAO = savedAlbum;
					break;
				}
			}
			if (updateTrack.exec()) {
				if (albumDAO != nullptr) {
					TrackDAO *trackDAO = new TrackDAO;
					trackDAO->setUri(oldPath);
					trackDAO->setTrackNumber(fh->trackNumber());
					trackDAO->setTitle(fh->title());
					trackDAO->setRating(rating);
					trackDAO->setDisc(QString::number(discNumber));
					trackDAO->setParentNode(albumDAO);
					emit nodeExtracted(trackDAO);
				}
			}
			olds.append(fh);
		} else {
			QString newPath = newPaths.at(i);
			QSqlQuery hasTrack("SELECT COUNT(*) FROM tracks WHERE uri = ?", *this);
			hasTrack.addBindValue(oldPath);
			if (hasTrack.exec() && hasTrack.next() && hasTrack.record().value(0).toInt() != 0) {
				QSqlQuery removeTrack("DELETE FROM tracks WHERE uri = ?", *this);
				removeTrack.addBindValue(oldPath);
				qDebug() << Q_FUNC_INFO << "deleting tracks";
				if (removeTrack.exec()) {
					this->saveFileRef(newPath);
				}
			}

			FileHelper *fh = new FileHelper(newPath);
			news.append(fh);
		}
	}
	if (this->cleanNodesWithoutTracks()) {
		// Finally, tell views they need to update themselves
		emit aboutToCleanView();
	}
	commit();
}

/** When one has manually updated tracks with TagEditor, some nodes might in unstable state. */
bool SqlDatabase::cleanNodesWithoutTracks()
{
	QSqlQuery albumsWithoutTracks("SELECT DISTINCT a.id FROM albums a WHERE a.id NOT IN (SELECT DISTINCT t.albumId FROM tracks t)", *this);
	if (albumsWithoutTracks.exec()) {
		while (albumsWithoutTracks.next()) {
			QSqlQuery deleteAlbum("DELETE FROM albums WHERE id = ?", *this);
			deleteAlbum.addBindValue(albumsWithoutTracks.record().value(0).toUInt());
			deleteAlbum.exec();
		}
	}

	QSqlQuery artistsWithoutTracks("SELECT DISTINCT a.id FROM artists a WHERE a.id NOT IN (SELECT DISTINCT t.artistId FROM tracks t)", *this);
	if (artistsWithoutTracks.exec()) {
		while (artistsWithoutTracks.next()) {
			QSqlQuery deleteArtist("DELETE FROM artists WHERE id = ?", *this);
			deleteArtist.addBindValue(artistsWithoutTracks.record().value(0).toUInt());
			deleteArtist.exec();
		}
	}
	return lastError().type() == QSqlError::NoError;
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
			// Cover path is now pointing to the first track of this album, because it need to be extracted at runtime
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
						albumDAO->setArtist(artistDAO->title());
						albumDAO->setId(QString::number(albumId));
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

	QSqlQuery cleanDb(*this);
	//cleanDb.exec("DELETE FROM tracks WHERE uri LIKE 'file:%'");
	//cleanDb.exec("DELETE FROM albums WHERE id NOT IN (SELECT DISTINCT albumId FROM tracks)");
	//cleanDb.exec("DELETE FROM artists WHERE id NOT IN (SELECT DISTINCT artistId FROM tracks)");
	cleanDb.exec("DELETE FROM tracks");
	cleanDb.exec("DELETE FROM albums");
	cleanDb.exec("DELETE FROM artists");
	cleanDb.exec("DROP INDEX indexArtist");
	cleanDb.exec("DROP INDEX indexAlbum");
	cleanDb.exec("DROP INDEX indexPath");
	cleanDb.exec("DROP INDEX indexArtistId");
	cleanDb.exec("DROP INDEX indexAlbumId");
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
	for (QString oldLocation : oldLocations) {
		if (newLocations.isEmpty() || !newLocations.contains(oldLocation)) {
			QSqlQuery syncDb(*this);
			syncDb.prepare("DELETE FROM tracks WHERE uri LIKE :path ");
			syncDb.bindValue(":path", "file://" + QDir::fromNativeSeparators(oldLocation) + "%");
			syncDb.exec();
			syncDb.exec("DELETE FROM albums WHERE id NOT IN (SELECT DISTINCT albumId FROM tracks)");
			syncDb.exec("DELETE FROM artists WHERE id NOT IN (SELECT DISTINCT artistId FROM tracks)");
		}
	}
	commit();

	// Restart the worker thread on new locations
	QStringList locationsToAdd;
	for (QString newLocation : newLocations) {
		if (!oldLocations.contains(newLocation)) {
			locationsToAdd.append(newLocation);
		}
	}

	if (locationsToAdd.isEmpty()) {
		this->load();
	} else {
		_musicSearchEngine->moveToThread(&_workerThread);
		_musicSearchEngine->doSearch(locationsToAdd);
	}
}

/** Load an existing database file or recreate it, if not found. */
void SqlDatabase::load()
{
	if (open() && tables().contains("tracks")) {
		qDebug() << Q_FUNC_INFO << sender();
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
		"disc, internalCover, rating) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

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
	insertTrack.addBindValue(fh.rating());

	bool artistInserted = false;
	bool albumInserted = false;
	ArtistDAO *artistDAO = nullptr;
	AlbumDAO *albumDAO = nullptr;

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
		QSqlQuery updateAlbum(*this);
		if (QString::compare(selectAlbum.record().value(0).toString(), album) == 0) {
			// Remote album with an icon in the treeview, then we add the exact same album from harddrive
			// for example, first: listenned in streaming, second: enjoyed, then downloaded (legit DL of course)
			updateAlbum.prepare("UPDATE albums SET host = nullptr, icon = nullptr WHERE id = ?");
		} else {
			// A previous record exists for this normalized name but the new name is different
			updateAlbum.prepare("UPDATE albums SET name = ? WHERE id = ?");
			updateAlbum.addBindValue(album);
		}
		updateAlbum.addBindValue(albumId);
		updateAlbum.exec();
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
		QSqlQuery updateArtist(*this);
		if (QString::compare(selectArtist.record().value(0).toString(), artistAlbum) == 0) {
			updateArtist.prepare("UPDATE artists SET host = nullptr WHERE id = ?");
		} else {
			updateArtist.prepare("UPDATE artists SET name = ?, host = nullptr WHERE id = ?");
			updateArtist.addBindValue(artistAlbum);
		}
		updateArtist.addBindValue(artistId);
		updateArtist.exec();
	}
}
