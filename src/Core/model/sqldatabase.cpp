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

SqlDatabase::SqlDatabase()
	: QObject()
	, QSqlDatabase("QSQLITE")
{
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
		//QTimer *t = new QTimer(this);
		//t->setSingleShot(true);
		//t->start(5000);
		//connect(t, &QTimer::timeout, this, &SqlDatabase::rebuild);
	}
	dbFile.open(QIODevice::ReadWrite);
	dbFile.close();
	setDatabaseName(dbPath);
}

SqlDatabase::~SqlDatabase()
{
	if (isOpen()) {
		close();
	}
}

void SqlDatabase::init()
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery createDb(*this);
	createDb.exec("CREATE TABLE IF NOT EXISTS cache (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, trackTitle varchar(255), trackLength INTEGER, " \
				  "artist varchar(255), artistNormalized varchar(255), " \
				  "album varchar(255), albumNormalized varchar(255), artistAlbum varchar(255), albumYear INTEGER,  " \
				  "rating INTEGER, disc INTEGER, cover varchar(255), internalCover INTEGER DEFAULT 0, host varchar(255), icon varchar(255))");

	createDb.exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, icon varchar(255), " \
				  "host varchar(255), background varchar(255), checksum varchar(255))");
	createDb.exec("CREATE TABLE IF NOT EXISTS playlistTracks (trackNumber INTEGER, title varchar(255), album varchar(255), length INTEGER, " \
				  "artist varchar(255), rating INTEGER, year INTEGER, icon varchar(255), host varchar(255), id INTEGER, " \
				  "url varchar(255), playlistId INTEGER, FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
	/// TEST Monitor Filesystem
	createDb.exec("CREATE TABLE IF NOT EXISTS filesystem (path VARCHAR(255) PRIMARY KEY ASC, " \
				  "lastModified INTEGER);");
}

bool SqlDatabase::insertIntoTableArtists(ArtistDAO *artist)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery insertArtist(*this);
	insertArtist.setForwardOnly(true);
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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	static std::uniform_int_distribution<uint> tt;
	this->transaction();
	uint id = 0;
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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery insertTrack(*this);
	insertTrack.prepare("INSERT INTO cache (uri, trackNumber, trackTitle, artist, album, artistAlbum, trackLength, rating, " \
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
	return b;
}

bool SqlDatabase::insertIntoTableTracks(const std::list<TrackDAO> &tracks)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}
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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	qDebug() << Q_FUNC_INFO << host;
	this->transaction();
	QSqlQuery removeTracks(*this);
	removeTracks.prepare("DELETE FROM cache WHERE host LIKE :h");
	removeTracks.bindValue(":h", host);
	removeTracks.exec();

	this->commit();
	/// FIXME
	//this->loadFromFileDB();
}

Cover* SqlDatabase::selectCoverFromURI(const QString &uri)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	Cover *c = nullptr;

	QSqlQuery selectCover(*this);
	selectCover.prepare("SELECT DISTINCT internalCover, cover FROM cache WHERE uri = ?");
	selectCover.addBindValue(uri);
	if (selectCover.exec() && selectCover.next()) {
		bool internalCover = selectCover.record().value(0).toBool();
		QString coverPath = selectCover.record().value(1).toString();
		QString album = selectCover.record().value(2).toString();
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
			selectCover.prepare("SELECT uri FROM cache WHERE album = ? AND internalCover = 1 LIMIT 1");
			selectCover.addBindValue(album);
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
	if (!isOpen()) {
		open();
		this->setPragmas();
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
			tracks.append(std::move(track));
		}
	}
	return tracks;
}

PlaylistDAO SqlDatabase::selectPlaylist(uint playlistId)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
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
	return playlist;
}

QList<PlaylistDAO> SqlDatabase::selectPlaylists()
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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

AlbumDAO* SqlDatabase::selectAlbumFromArtist(ArtistDAO *artistDAO, QString album)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery selectAlbum(*this);
	selectAlbum.prepare("SELECT album, normalizedName, year, cover, icon, host FROM cache WHERE album = ?");
	selectAlbum.addBindValue(album);
	if (selectAlbum.exec() && selectAlbum.next()) {
		AlbumDAO *album = new AlbumDAO;
		int i = -1;
		album->setId(selectAlbum.record().value(++i).toString());
		album->setTitle(selectAlbum.record().value(++i).toString());
		album->setTitleNormalized(selectAlbum.record().value(++i).toString());
		album->setYear(selectAlbum.record().value(++i).toString());
		album->setCover(selectAlbum.record().value(++i).toString());
		album->setIcon(selectAlbum.record().value(++i).toString());
		album->setHost(selectAlbum.record().value(++i).toString());
		if (artistDAO) {
			album->setArtist(artistDAO->title());
		}
		return album;
	} else {
		return nullptr;
	}
}

ArtistDAO* SqlDatabase::selectArtist(uint artistId)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	TrackDAO track;
	QSqlQuery qTracks(*this);
	qTracks.prepare("SELECT uri, trackNumber, trackTitle, artist, album, artistAlbum, trackLength, " \
					"rating, disc, internalCover, host, icon, albumYear " \
					"FROM cache WHERE uri = ?");
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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery query = exec("SELECT background FROM playlists WHERE id = " + QString::number(playlistID));
	query.next();
	bool result = !query.record().value(0).toString().isEmpty();
	qDebug() << Q_FUNC_INFO << query.record().value(0).toString() << result;
	return result;
}

bool SqlDatabase::updateTablePlaylist(const PlaylistDAO &playlist)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery update(*this);
	update.prepare("UPDATE playlists SET title = ?, checksum = ? WHERE id = ?");
	update.addBindValue(playlist.title());
	update.addBindValue(playlist.checksum());
	update.addBindValue(playlist.id());
	return update.exec();
}

void SqlDatabase::updateTablePlaylistWithBackgroundImage(uint playlistID, const QString &backgroundImagePath)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery update(*this);
	update.prepare("UPDATE playlists SET background = ? WHERE id = ?");
	update.addBindValue(backgroundImagePath);
	update.addBindValue(playlistID);
	update.exec();
}

void SqlDatabase::updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

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
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	// Signals are blocked to prevent saveFileRef method to emit one. Load method will tell connected views to rebuild themselves
	transaction();
	Q_ASSERT(oldPaths.size() == newPaths.size());

	qDebug() << Q_FUNC_INFO << "oldPaths" << oldPaths;
	qDebug() << Q_FUNC_INFO << "newPaths" << newPaths;

	QList<QUrl> olds;
	QList<ArtistDAO*> artists;
	QList<AlbumDAO*> albums;

	// If New Path exists, then fileName has changed.
	/// TODO

	QList<QUrl> news;
	emit aboutToUpdateView(olds, news);
	commit();
}

/** Delete cache and rescan local tracks. */
void SqlDatabase::rebuild()
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QSqlQuery cleanDb(*this);
	cleanDb.setForwardOnly(true);
	cleanDb.exec("DELETE FROM cache");
	cleanDb.exec("DROP INDEX indexUri");
	transaction();
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

	QSqlQuery updateCoverPath("UPDATE cache SET cover = ? WHERE uri = ?", *this);
	updateCoverPath.setForwardOnly(true);
	updateCoverPath.addBindValue(coverPath);
	updateCoverPath.addBindValue(track);
	bool b = updateCoverPath.exec();
	if (b) {
		QSqlQuery selectAlbum("SELECT DISTINCT id, albumName, albumNormalized, year, cover, artist FROM cache WHERE uri = ?", *this);
		selectAlbum.setForwardOnly(true);
		selectAlbum.addBindValue(track);
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

void SqlDatabase::setPragmas()
{
	this->exec("PRAGMA journal_mode = MEMORY");
	this->exec("PRAGMA synchronous = OFF");
	this->exec("PRAGMA temp_store = 2");
	this->exec("PRAGMA foreign_keys = 1");
}

/** Reads a file from the filesystem and adds it into the library. */
void SqlDatabase::saveFileRef(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	if (!fh.isValid()) {
		qDebug() << Q_FUNC_INFO << "file is not valid, won't be saved";
		return;
	}

	QSqlQuery insertTrack(*this);
	insertTrack.setForwardOnly(true);
	insertTrack.prepare("INSERT INTO cache (uri, trackNumber, trackTitle, artist, artistNormalized, album, albumNormalized, " \
						"albumYear, artistAlbum, trackLength, disc, internalCover, rating) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	QString tn = fh.trackNumber();
	QString title = fh.title();
	QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();

	// Use Artist Album to reference tracks in table "tracks", not Artist
	QString artistNorm = this->normalizeField(artistAlbum);
	QString album = fh.album();
	QString albumNorm = this->normalizeField(album);
	QString length = fh.length();
	//uint artistId = qHash(artistNorm);
	//uint albumId = artistId + qHash(albumNorm, 1);
	int dn = fh.discNumber();

	insertTrack.addBindValue(absFilePath);
	insertTrack.addBindValue(tn.toInt());
	if (title.isEmpty()) {
		insertTrack.addBindValue(fh.fileInfo().baseName());
	} else {
		insertTrack.addBindValue(title);
	}
	insertTrack.addBindValue(fh.artist());
	insertTrack.addBindValue(artistNorm);
	insertTrack.addBindValue(fh.album());
	insertTrack.addBindValue(albumNorm);
	insertTrack.addBindValue(fh.year());
	insertTrack.addBindValue(artistAlbum);
	insertTrack.addBindValue(length);
	insertTrack.addBindValue(dn);
	insertTrack.addBindValue(fh.hasCover());
	insertTrack.addBindValue(fh.rating());

	//bool albumInserted = false;
	//ArtistDAO *artistDAO = nullptr;
	//AlbumDAO *albumDAO = nullptr;

	if (!insertTrack.exec()) {
		qDebug() << Q_FUNC_INFO << insertTrack.lastError();
		return;
	}
	/*TrackDAO *trackDAO = new TrackDAO;
	trackDAO->setUri(absFilePath);
	trackDAO->setTrackNumber(tn);
	trackDAO->setTitle(title);
	trackDAO->setArtist(fh.artist());
	trackDAO->setAlbum(album);
	trackDAO->setArtistAlbum(artistAlbum);
	trackDAO->setLength(length);
	trackDAO->setRating(fh.rating());
	trackDAO->setDisc(QString::number(dn));*/

	QSqlQuery selectAlbum(*this);
	selectAlbum.setForwardOnly(true);
	selectAlbum.prepare("SELECT albumName, normalizedName FROM cache WHERE albumName = ? AND artistName = ?");
	selectAlbum.addBindValue(fh.album());
	selectAlbum.addBindValue(fh.artist());
	selectAlbum.exec();
	if (!selectAlbum.next()) {
		/*QSqlQuery insertAlbum(*this);
		insertAlbum.setForwardOnly(true);
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
		}*/
		//_cache.insert(albumId, albumDAO);

	} else {
		/*QSqlQuery updateAlbum(*this);
		updateAlbum.setForwardOnly(true);
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
		updateAlbum.exec();*/
	}
	/*QSqlQuery selectArtist(*this);
	selectArtist.setForwardOnly(true);
	selectArtist.prepare("SELECT name, normalizedName FROM artists WHERE id = ?");
	selectArtist.addBindValue(artistId);
	selectArtist.exec();
	if (!selectArtist.next()) {
		QSqlQuery insertArtist(*this);
		insertArtist.setForwardOnly(true);
		insertArtist.prepare("INSERT INTO artists (id, name, normalizedName) VALUES (?, ?, ?)");
		insertArtist.addBindValue(artistId);
		insertArtist.addBindValue(artistAlbum);
		insertArtist.addBindValue(artistNorm);
		insertArtist.exec();

		artistDAO = new ArtistDAO;
		artistDAO->setId(QString::number(artistId));
		artistDAO->setTitle(artistAlbum);
		artistDAO->setTitleNormalized(artistNorm);
		_cache.insert(artistId, artistDAO);
	} else {
		QSqlQuery updateArtist(*this);
		updateArtist.setForwardOnly(true);
		if (QString::compare(selectArtist.record().value(0).toString(), artistAlbum) == 0) {
			updateArtist.prepare("UPDATE artists SET host = nullptr WHERE id = ?");
		} else {
			updateArtist.prepare("UPDATE artists SET name = ?, host = nullptr WHERE id = ?");
			updateArtist.addBindValue(artistAlbum);
		}
		updateArtist.addBindValue(artistId);
		updateArtist.exec();
	}*/
}
