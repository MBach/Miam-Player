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

	//connect(_musicSearchEngine, &MusicSearchEngine::scannedFiled, this, &LibrarySqlModel::readFile);
	connect(_musicSearchEngine, &MusicSearchEngine::scannedFiled, [=](const QString &file) {
		this->readFile(file);
	});

	// When the scan is complete, save the model in the filesystem
	connect(_musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, &LibrarySqlModel::saveDB);

}

void LibrarySqlModel::loadFromFile()
{
	_db.open();

	QSqlQuery qLoadFileDB = _db.exec("SELECT * FROM tracks;");
	qDebug() << qLoadFileDB.lastError() << qLoadFileDB.size();
	while (qLoadFileDB.next()) {
		QString s;
		QSqlRecord r = qLoadFileDB.record();
		for (int i = 0; i < r.count(); i++) {
			s.append(r.field(i).value().toString());
			s.append(" ");
		}
		//qDebug() << "load from hdd: " << s;
	}

	_db.close();
}

void LibrarySqlModel::rebuild()
{
	// Open Connection
	qDebug() << "db open" << _db.open();

	_db.exec("DROP TABLE tracks");

	// Create a table in the memory DB
	QSqlQuery qCreateTable = _db.exec("CREATE TABLE tracks (artist varchar(255), artistAlbum varchar(255), album varchar(255), title varchar(255), absPath varchar(255))");
	qDebug() << "create: " << qCreateTable.lastError().type();

	// Foreach file, insert tuple
	_musicSearchEngine->doSearch();
}

/** Reads a file from the filesystem and adds it into the library. */
void LibrarySqlModel::readFile(const QString &absFilePath)
{
	qDebug() << Q_FUNC_INFO << absFilePath;
	FileHelper fh(absFilePath);
	if (fh.isValid()) {
		QSqlQuery insert;
		insert.prepare("INSERT INTO tracks (artist, artistAlbum, album, title, absPath) VALUES (?, ?, ?, ?, ?);");
		insert.addBindValue(fh.artist());
		insert.addBindValue(fh.artistAlbum());
		insert.addBindValue(fh.album());
		insert.addBindValue(fh.title());
		insert.addBindValue(absFilePath);
		insert.exec();

		//QPersistentModelIndex p();

		QSqlError e = insert.lastError();
		//qDebug() << "Error ?" << (e.type() != 0);

		/*fh.artist();
		fh.title();
		absFilePath;
		fh.album();
		fh.artistAlbum();
		fh.discNumber();
		fh.trackNumber().toInt();
		fh.year().toInt();*/
		//qDebug() << "about to tell view that" << absFilePath << "was stored in a local database";
		emit trackCreated(fh);
	} else {
		//qDebug() << "INVALID FILE FOR:" << absFilePath;
	}
}

void LibrarySqlModel::saveDB()
{
	// Close Connection
	_db.close();
}
