#include "musicsearchengine.h"
#include "filehelper.h"
#include "settingsprivate.h"
#include "model/sqldatabase.h"

#include <QDateTime>
#include <QDirIterator>
#include <QFileInfo>
#include <QThread>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QtDebug>

bool MusicSearchEngine::isScanning = false;

void MusicSearchEngine::setDelta(const QStringList &delta)
{
	_delta = delta;
}

MusicSearchEngine::MusicSearchEngine(QObject *parent)
	: QObject(parent)
	, _timer(new QTimer(this))
{
	_timer->setInterval(5000);
	connect(_timer, &QTimer::timeout, this, &MusicSearchEngine::watchForChanges);

	// Monitor filesystem
	if (SettingsPrivate::instance()->isFileSystemMonitored()) {
		setWatchForChanges(true);
	}
}

void MusicSearchEngine::setWatchForChanges(bool b)
{
	if (b) {
		_timer->start();
	} else {
		_timer->stop();
	}
}

void MusicSearchEngine::doSearch()
{
	qDebug() << Q_FUNC_INFO;
	MusicSearchEngine::isScanning = true;
	QList<QDir> locations;
	QStringList pathsToSearch = _delta.isEmpty() ? SettingsPrivate::instance()->musicLocations() : _delta;
	for (QString musicPath : pathsToSearch) {
		QDir location(musicPath);
		location.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
		locations.append(location);
	}

	int entryCount = 0;
	// QDirIterator class is very fast to scan large directories
	for (QDir location : locations) {
		QDirIterator it(location.absolutePath(), QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			entryCount++;
		}
	}

	int currentEntry = 0;
	int percent = 1;
	bool atLeastOneAudioFileWasFound = false;
	bool isNewDirectory = false;

	QString coverPath;
	QString lastFileScannedNextToCover;

	QStringList suffixes = FileHelper::suffixes(FileHelper::All);

	for (QDir location : locations) {
		QDirIterator it(location.absolutePath(), QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString entry = it.next();
			QFileInfo qFileInfo(entry);
			currentEntry++;

			// Directory has changed: we can discard cover
			if (qFileInfo.isDir()) {
				if (!coverPath.isEmpty() && !lastFileScannedNextToCover.isEmpty()) {
					emit scannedCover(coverPath, lastFileScannedNextToCover);
					coverPath.clear();
				}
				isNewDirectory = true;
				atLeastOneAudioFileWasFound = false;
				lastFileScannedNextToCover.clear();
				continue;
			} else if (qFileInfo.suffix().toLower() == "jpg" || qFileInfo.suffix().toLower() == "png") {
				if (atLeastOneAudioFileWasFound) {
					coverPath = qFileInfo.absoluteFilePath();
					emit scannedCover(coverPath, lastFileScannedNextToCover);
					coverPath.clear();
				} else if (isNewDirectory) {
					coverPath = qFileInfo.absoluteFilePath();
				}
			} else if (suffixes.contains(qFileInfo.suffix())) {
				emit scannedFile(qFileInfo.absoluteFilePath());
				atLeastOneAudioFileWasFound = true;
				lastFileScannedNextToCover = qFileInfo.absoluteFilePath();
				isNewDirectory = false;
			}

			if (currentEntry * 100 / entryCount > percent) {
				percent = currentEntry * 100 / entryCount;
				//qDebug() << "about to emit progress changed" << percent << "%";
				emit progressChanged(percent);
				qApp->processEvents();
			}
		}
		atLeastOneAudioFileWasFound = false;
	}
	emit searchHasEnded();
	MusicSearchEngine::isScanning = false;
}

void MusicSearchEngine::watchForChanges()
{
	// Gather all folders registered on music locations
	QFileInfoList dirs;
	for (QString musicPath : SettingsPrivate::instance()->musicLocations()) {
		QFileInfo location(musicPath);
		QDirIterator it(location.absoluteFilePath(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString entry = it.next();
			QFileInfo qFileInfo(entry);
			dirs << qFileInfo;
		}
	}

	SqlDatabase db;// = SqlDatabase::instance();
	db.open();
	db.exec("PRAGMA journal_mode = MEMORY");
	db.exec("PRAGMA synchronous = OFF");
	db.exec("PRAGMA temp_store = 2");
	db.exec("PRAGMA foreign_keys = 1");

	QStringList newFoldersToAddInLibrary;
	// Add folders that were not found first
	for (QFileInfo f : dirs) {
		QSqlQuery query(db);
		query.setForwardOnly(true);
		query.prepare("SELECT * FROM filesystem WHERE path = ?");
		query.addBindValue(f.absoluteFilePath());
		if (query.exec() && !query.next()) {
			newFoldersToAddInLibrary << f.absoluteFilePath();
			QSqlQuery prepared(db);
			prepared.setForwardOnly(true);
			prepared.prepare("INSERT INTO filesystem (path, lastModified) VALUES (?, ?)");
			prepared.addBindValue(f.absoluteFilePath());
			prepared.addBindValue(f.lastModified().toTime_t());
			prepared.exec();
		}
	}

	if (!newFoldersToAddInLibrary.isEmpty()) {
		_delta = newFoldersToAddInLibrary;
		this->doSearch();
		//this->doSearch(newFoldersToAddInLibrary);
	}

	// Process in reverse mode to clean cache: from database file and check if entry exists in database
	QSqlQuery cache("SELECT * FROM filesystem", db);
	qDebug() << Q_FUNC_INFO << "SELECT * FROM filesystem";
	cache.setForwardOnly(true);
	if (cache.exec()) {
		QStringList oldLocations;
		while (cache.next()) {
			QDir d(cache.record().value(0).toString());
			d.exists();
			QFileInfo fileInfo(cache.record().value(0).toString());
			// Remove folder in database because it couldn't be find in the filesystem
			if (!fileInfo.exists()) {
				QSqlQuery deleteFromFilesystem(db);
				deleteFromFilesystem.prepare("DELETE FROM filesystem WHERE path = ?");
				deleteFromFilesystem.addBindValue(fileInfo.absoluteFilePath());
				qDebug() << Q_FUNC_INFO << "DELETE FROM filesystem WHERE path = ?";

				if (deleteFromFilesystem.exec()) {
					oldLocations << fileInfo.absoluteFilePath();
				}
			}
		}
		qDebug() << Q_FUNC_INFO << oldLocations;
		if (!oldLocations.isEmpty()) {
			//db.rebuildFomLocations(oldLocations, QStringList());
			db.musicSearchEngine()->setDelta(oldLocations);
			db.rebuild();
		}
	}
}
