#include "librarysqlmodel.h"
#include "filehelper.h"

#include <QtDebug>

LibrarySqlModel::LibrarySqlModel(QObject *parent) :
	QSqlTableModel(parent)
{
	_db = QSqlDatabase::addDatabase("QSQLITE");
	QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
	QString dbPath = QDir::toNativeSeparators(path + "/mmmmp.db");
	_db.setDatabaseName(dbPath);

	_musicSearchEngine = new MusicSearchEngine(this);

	connect(_musicSearchEngine, &MusicSearchEngine::scannedFiled, this, &LibrarySqlModel::readFile);

	// When the scan is complete, save the model in the filesystem
	connect(_musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, &LibrarySqlModel::saveDB);
}

void LibrarySqlModel::loadFromFileDB()
{
	this->beginResetModel();
	qDebug() << "db open success?" << _db.open();

	QSqlQuery qLoadFileDB;
	bool e = qLoadFileDB.exec("SELECT * FROM tracks");
	qDebug() << "exec success?" << e;
	while (qLoadFileDB.next()) {
		emit trackExtractedFromDB(qLoadFileDB.record());
	}
	_db.close();
	this->endResetModel();
}

void LibrarySqlModel::rebuild()
{
	this->beginResetModel();

	// Open Connection
	qDebug() << "db open" << _db.open();

	_db.exec("DROP TABLE tracks");

	// Create a table in the memory DB
	QSqlQuery qCreateTable = _db.exec("CREATE TABLE tracks (artist varchar(255), artistAlbum varchar(255), album varchar(255), title varchar(255), discNumber int, year int, absPath varchar(255))");
	qDebug() << "create: " << qCreateTable.lastError().type();

	// Foreach file, insert tuple
	_musicSearchEngine->doSearch();
}

/** Reads a file from the filesystem and adds it into the library. */
void LibrarySqlModel::readFile(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	if (fh.isValid()) {
		QSqlQuery insert;
		insert.prepare("INSERT INTO tracks (absPath, album, artist, artistAlbum, discNumber, title, year) VALUES (?, ?, ?, ?, ?, ?, ?)");
		insert.addBindValue(absFilePath);
		insert.addBindValue(fh.album());
		insert.addBindValue(fh.artist());
		insert.addBindValue(fh.artistAlbum());
		insert.addBindValue(fh.discNumber());
		insert.addBindValue(fh.title());
		insert.addBindValue(fh.year());
		insert.exec();		
		emit trackExtractedFromFS(fh);
	} else {
		//qDebug() << "INVALID FILE FOR:" << absFilePath;
	}
}

void LibrarySqlModel::saveDB()
{
	// Close Connection
	_db.close();
	this->endResetModel();
}
