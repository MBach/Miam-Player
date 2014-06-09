#include "librarysqlmodel.h"
#include "filehelper.h"
#include "musicsearchengine.h"

#include <cover.h>

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>

#include <QtDebug>

LibrarySqlModel::LibrarySqlModel(const QSqlDatabase &db, QObject *parent) :
	QSqlTableModel(parent, db), _musicSearchEngine(new MusicSearchEngine())
{
	connect(_musicSearchEngine, &MusicSearchEngine::progressChanged, this, &LibrarySqlModel::progressChanged);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedCover, this, &LibrarySqlModel::saveCoverRef);
	//connect(_musicSearchEngine, &MusicSearchEngine::scannedFile, this, &LibrarySqlModel::saveFileRef);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedFile, this, &LibrarySqlModel::saveFileRef);

	// When the scan is complete, save the model in the filesystem
	connect(_musicSearchEngine, &MusicSearchEngine::searchHasEnded, [=] () {
		database().exec("COMMIT TRANSACTION");
		database().exec("CREATE INDEX indexArtist ON tracks (artist)");
		database().exec("CREATE INDEX indexAlbum ON tracks (album)");
		database().exec("CREATE INDEX indexArtistAlbum ON tracks (artistAlbum)");
		database().exec("CREATE INDEX indexPath ON tracks (path)");
		// Close Connection
		database().close();
		this->endResetModel();
	});
}

void LibrarySqlModel::updateLibrary(const QStringList &oldTracks, const QStringList &newTracks)
{
	if (!database().isOpen()) {
		database().open();
	}
	database().driver()->beginTransaction();

	foreach (QString track, oldTracks) {
		qDebug() << "updateLibrary, removing" << track;
		// "absPath" is the primary key
		QSqlQuery removeTracks(database());
		removeTracks.prepare("DELETE FROM tracks WHERE absPath = :track");
		removeTracks.addBindValue(track);
		removeTracks.exec();
	}

	// Replace tracks in the database
	foreach (QString track, newTracks) {
		qDebug() << "updateLibrary, inserting" << track;
		this->saveFileRef(track, false);
	}
	database().driver()->commitTransaction();
	database().close();

	/// XXX: might not be the smartest way to reload changes, but it's way simpler than searching in a tree for modifications
	this->loadFromFileDB();
}

void LibrarySqlModel::loadFromFileDB()
{
	this->beginResetModel();
	if (!database().isOpen()) {
		database().open();
	}

	QSqlQuery qLoadFileDB = database().exec("SELECT * FROM tracks");
	while (qLoadFileDB.next()) {
		emit trackExtractedFromDB(qLoadFileDB.record());
	}
	database().close();
	this->endResetModel();
}

void LibrarySqlModel::rebuild()
{
	this->beginResetModel();

	// Open Connection
	database().open();

	database().exec("PRAGMA synchronous = OFF");
	database().exec("PRAGMA journal_mode = MEMORY");
	database().exec("DROP TABLE tracks");
	QString createTable("CREATE TABLE tracks (artist varchar(255), artistAlbum varchar(255), album varchar(255), ");
	createTable.append("title varchar(255), trackNumber INTEGER, discNumber INTEGER, year INTEGER, absPath varchar(255) PRIMARY KEY ASC, ");
	createTable.append("path varchar(255), coverAbsPath varchar(255), internalCover INTEGER DEFAULT 0)");
	database().exec(createTable);

	database().exec("BEGIN TRANSACTION");

	// Foreach file, insert tuple
	_musicSearchEngine->moveToThread(&_workerThread);
	_musicSearchEngine->doSearch();
}

void LibrarySqlModel::load()
{
	if (database().open() && database().tables().contains("tracks")) {
		this->loadFromFileDB();
	} else {
		this->rebuild();
	}
}

/** Reads an external picture which is close to multimedia files (same folder). */
void LibrarySqlModel::saveCoverRef(const QString &coverPath)
{
	QFileInfo fileInfo(coverPath);
	QSqlQuery updateCoverPath("UPDATE tracks SET coverAbsPath = ? WHERE path = ?", database());
	updateCoverPath.addBindValue(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
	updateCoverPath.addBindValue(QDir::toNativeSeparators(fileInfo.absolutePath()));
	updateCoverPath.exec();
	emit coverWasUpdated(fileInfo);
}

/** Reads a file from the filesystem and adds it into the library. */
void LibrarySqlModel::saveFileRef(const QString &absFilePath, bool emitSignal)
{
	FileHelper fh(absFilePath);
	if (fh.isValid()) {
		QSqlQuery insert(database());
		insert.prepare("INSERT INTO tracks (absPath, album, artist, artistAlbum, discNumber, internalCover, path, title, trackNumber, year) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		insert.addBindValue(QDir::toNativeSeparators(absFilePath));
		insert.addBindValue(fh.album());
		insert.addBindValue(fh.artist());
		insert.addBindValue(fh.artistAlbum());
		insert.addBindValue(fh.discNumber());
		insert.addBindValue(fh.hasCover());
		insert.addBindValue(QDir::toNativeSeparators(fh.fileInfo().absolutePath()));
		insert.addBindValue(fh.title());
		insert.addBindValue(fh.trackNumber().toInt());
		insert.addBindValue(fh.year().toInt());
		insert.exec();
		if (emitSignal) {
			emit trackExtractedFromFS(fh);
		}
	} else {
		qDebug() << "INVALID FILE FOR:" << absFilePath;
	}
}
