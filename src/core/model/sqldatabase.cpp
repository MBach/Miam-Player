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

#include <chrono>
#include <random>

SqlDatabase::SqlDatabase(QObject *parent)
	: QObject(parent)
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
	}

	setDatabaseName(dbPath);

	// DB folder exists but DB file doesn't: can be first launch or file was deleted manually
	if (dbFile.exists()) {
		this->init();
	} else {

		dbFile.open(QIODevice::ReadWrite);
		dbFile.close();

		this->init();

		QSqlQuery createDb(*this);
		createDb.exec("CREATE TABLE IF NOT EXISTS cache (uri varchar(255) PRIMARY KEY ASC, trackNumber INTEGER, trackTitle varchar(255), trackLength INTEGER, " \
					  "artist varchar(255), artistNormalized varchar(255), " \
					  "album varchar(255), albumNormalized varchar(255), artistAlbum varchar(255), albumYear INTEGER,  " \
					  "rating INTEGER, disc INTEGER, cover varchar(255), internalCover varchar(255), host varchar(255), icon varchar(255))");

		createDb.exec("CREATE TABLE IF NOT EXISTS playlists (id INTEGER PRIMARY KEY, title varchar(255), duration INTEGER, icon varchar(255), " \
					  "host varchar(255), background varchar(255), checksum varchar(255))");
		createDb.exec("CREATE TABLE IF NOT EXISTS playlistTracks (uri varchar(255) PRIMARY KEY ASC, playlistId INTEGER, FOREIGN KEY(playlistId) REFERENCES playlists(id) ON DELETE CASCADE)");
		/// TEST Monitor Filesystem
		createDb.exec("CREATE TABLE IF NOT EXISTS filesystem (path VARCHAR(255) PRIMARY KEY ASC, " \
					  "lastModified INTEGER);");

		// Wait for a few seconds and restart full scan
		/// TODO: full rescan <> rebuild which is only for local tracks
		/// Remote tracks (like Deezer) are still not synchronized
		//QTimer *t = new QTimer(this);
		//t->setSingleShot(true);
		//t->start(5000);
		//connect(t, &QTimer::timeout, this, &SqlDatabase::rebuild);
	}

}

SqlDatabase::~SqlDatabase()
{
	if (isOpen()) {
		close();
	}
}

void SqlDatabase::reset()
{
	exec("DELETE FROM cache");
	exec("DROP INDEX indexArtist");
	exec("DROP INDEX indexAlbum");
	exec("DROP INDEX indexPath");
}

void SqlDatabase::init()
{
	open();
	this->setPragmas();
}

uint SqlDatabase::insertIntoTablePlaylists(const PlaylistDAO &playlist, const QStringList &tracks, bool isOverwriting)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	qDebug() << Q_FUNC_INFO << tracks;

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

bool SqlDatabase::insertIntoTablePlaylistTracks(uint playlistId, const QStringList &tracks, bool isOverwriting)
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
	/// TODO remote tracks?
	for (QString track : tracks) {
		QSqlQuery insert(*this);
		insert.prepare("INSERT INTO playlistTracks (uri, playlistId) VALUES (?, ?)");
		insert.addBindValue(track);
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
		QString internalCover = selectCover.record().value(0).toString();
		QString coverPath = selectCover.record().value(1).toString();
		QString album = selectCover.record().value(2).toString();
		if (!internalCover.isEmpty() || !coverPath.isEmpty()) {
			// If URI has an internal cover, i.e. uri points to a local file
			if (internalCover.isEmpty()) {
				c = new Cover(coverPath);
			} else {
				FileHelper fh(uri);
				c = fh.extractCover();
			}
		} else {
			// No direct cover for this file, let's search for the entire album if one track has an inner cover
			selectCover.prepare("SELECT uri FROM cache WHERE album = ? AND internalCover <> NULL LIMIT 1");
			selectCover.addBindValue(album);
			if (selectCover.exec() && selectCover.next()) {
				FileHelper fh(selectCover.record().value(0).toString());
				c = fh.extractCover();
			}
		}
	}
	return c;
}

QStringList SqlDatabase::selectPlaylistTracks(uint playlistID, bool withPrefix)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	QStringList tracks;
	QSqlQuery results(*this);
	results.prepare("SELECT uri FROM playlistTracks WHERE playlistId = ?");
	results.addBindValue(playlistID);
	if (results.exec()) {
		while (results.next()) {
			QSqlRecord record = results.record();
			if (withPrefix) {
				tracks << "file://" + record.value(0).toString();
			} else {
				tracks << record.value(0).toString();
			}
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

/*ArtistDAO* SqlDatabase::selectArtist(uint artistId)
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
}*/

TrackDAO SqlDatabase::selectTrackByURI(const QString &uri)
{
	if (!isOpen()) {
		open();
		this->setPragmas();
	}

	TrackDAO track;
	QSqlQuery qTracks(*this);
	qTracks.prepare("SELECT uri, trackNumber, trackTitle, artist, album, artistAlbum, trackLength, " \
					"rating, disc, host, icon, albumYear " \
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


void SqlDatabase::updateTrack(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	if (!fh.isValid()) {
		qDebug() << Q_FUNC_INFO << "file is not valid, won't be updated";
		return;
	}

	QSqlQuery updateTrack(*this);
	updateTrack.setForwardOnly(true);
	updateTrack.prepare("UPDATE cache SET trackNumber = ?, trackTitle = ?, artist = ?, artistNormalized = ?, album = ?, albumNormalized = ?, " \
						"albumYear = ?, artistAlbum = ?, trackLength = ?, disc = ?, internalCover = ?, rating = ? WHERE uri = ?");

	QString tn = fh.trackNumber();
	QString title = fh.title();
	QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();

	// Use Artist Album to reference tracks in table "tracks", not Artist
	QString artistNorm = this->normalizeField(artistAlbum);
	QString album = fh.album();
	QString albumNorm = this->normalizeField(album);
	QString length = fh.length();
	int dn = fh.discNumber();

	updateTrack.addBindValue(tn.toInt());
	if (title.isEmpty()) {
		updateTrack.addBindValue(fh.fileInfo().baseName());
	} else {
		updateTrack.addBindValue(title);
	}
	updateTrack.addBindValue(fh.artist());
	updateTrack.addBindValue(artistNorm);
	updateTrack.addBindValue(fh.album());
	updateTrack.addBindValue(albumNorm);
	updateTrack.addBindValue(fh.year());
	updateTrack.addBindValue(artistAlbum);
	updateTrack.addBindValue(length);
	updateTrack.addBindValue(dn);
	if (fh.hasCover()) {
		updateTrack.addBindValue(absFilePath);
	} else {
		updateTrack.addBindValue(QVariant());
	}
	updateTrack.addBindValue(fh.rating());
	updateTrack.addBindValue(absFilePath);

	if (!updateTrack.exec()) {
		qDebug() << Q_FUNC_INFO << updateTrack.lastError();
	}
}

/** Update a list of tracks. If track name has changed, will be removed from Library then added right after. */
void SqlDatabase::updateTracks(const QStringList &oldPaths, const QStringList &newPaths)
{
	this->init();

	// Signals are blocked to prevent saveFileRef method to emit one. Load method will tell connected views to rebuild themselves
	transaction();
	Q_ASSERT(oldPaths.size() == newPaths.size());

	//qDebug() << Q_FUNC_INFO << "oldPaths" << oldPaths;
	//qDebug() << Q_FUNC_INFO << "newPaths" << newPaths;
	for (int i = 0; i < newPaths.size(); i++) {
		QString newPath = newPaths.at(i);
		QString oldPath = oldPaths.at(i);
		if (newPath.isEmpty()) {
			this->updateTrack(oldPath);
		} else {

			QSqlQuery removeTrack(*this);
			removeTrack.prepare("DELETE FROM cache WHERE uri = :h");
			removeTrack.bindValue(":h", oldPath);
			removeTrack.exec();

			this->saveFileRef(newPath);
		}
	}

	commit();
	emit aboutToUpdateView();
}

/** Reads an external picture which is close to multimedia files (same folder). */
void SqlDatabase::saveCoverRef(const QString &coverPath, const QString &track)
{
	FileHelper fh(track);
	QString artistAlbum = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();
	QString artistNorm = this->normalizeField(artistAlbum);
	QString albumNorm = this->normalizeField(fh.album());

	QSqlQuery updateCoverPath("UPDATE cache SET cover = ? WHERE artistNormalized = ? AND albumNormalized = ?", *this);
	updateCoverPath.setForwardOnly(true);
	updateCoverPath.addBindValue(coverPath);
	updateCoverPath.addBindValue(artistNorm);
	updateCoverPath.addBindValue(albumNorm);
	updateCoverPath.exec();
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
	this->exec("PRAGMA journal_mode = OFF");
	this->exec("PRAGMA synchronous = OFF");
	this->exec("PRAGMA temp_store = 2");
	this->exec("PRAGMA foreign_keys = 1");
	this->exec("PRAGMA count_changes = OFF");
}

/** Reads a file from the filesystem and adds it into the library. */
void SqlDatabase::saveFileRef(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	if (!fh.isValid()) {
		qDebug() << Q_FUNC_INFO << "file is not valid, won't be saved:" << absFilePath;
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
	if (fh.hasCover()) {
		insertTrack.addBindValue(absFilePath);
	} else {
		insertTrack.addBindValue(QVariant());
	}
	insertTrack.addBindValue(fh.rating());

	if (!insertTrack.exec()) {
		qDebug() << Q_FUNC_INFO << insertTrack.lastError();
	}
}
