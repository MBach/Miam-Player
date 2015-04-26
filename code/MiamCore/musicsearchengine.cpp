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

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
	QObject(parent), _timer(new QTimer(this))
{
	//_timer->setInterval(5 * 60 * 1000);
	_timer->setInterval(5000);
	/// debug
	///_timer->setSingleShot(true);
	connect(_timer, &QTimer::timeout, this, &MusicSearchEngine::watchForChanges);

	// Monitor filesystem
	if (SettingsPrivate::instance()->isFileSystemMonitored()) {
		qDebug() << Q_FUNC_INFO;
		setWatchForChanges(true);
	} else {
		qDebug() << Q_FUNC_INFO;
	}
}

void MusicSearchEngine::setWatchForChanges(bool b)
{
	qDebug() << Q_FUNC_INFO << b << MusicSearchEngine::isScanning;
	if (b) {
		_timer->start();
	} else {
		_timer->stop();
	}
}

void MusicSearchEngine::doSearch(const QStringList &delta)
{
	qDebug() << Q_FUNC_INFO << delta;
	MusicSearchEngine::isScanning = true;
	QList<QDir> locations;
	QStringList pathsToSearch = delta.isEmpty() ? SettingsPrivate::instance()->musicLocations() : delta;
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
			} else {
				emit scannedFile(qFileInfo.absoluteFilePath());
				atLeastOneAudioFileWasFound = true;
				lastFileScannedNextToCover = qFileInfo.absoluteFilePath();
				isNewDirectory = false;
			}

			if (currentEntry * 100 / entryCount > percent) {
				percent = currentEntry * 100 / entryCount;
				emit progressChanged(percent);
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

	SqlDatabase *db = SqlDatabase::instance();
	QStringList newFoldersToAddInLibrary;
	// Add folders that were not found first
	for (QFileInfo f : dirs) {
		QSqlQuery query = db->exec("SELECT * FROM filesystem WHERE path = \"" + f.absoluteFilePath() + "\"");
		if (!query.next()) {
			newFoldersToAddInLibrary << f.absoluteFilePath();
			QSqlQuery prepared(*db);
			prepared.prepare("INSERT INTO filesystem (path, lastModified) VALUES (?, ?)");
			prepared.addBindValue(f.absoluteFilePath());
			prepared.addBindValue(f.lastModified().toTime_t());
			prepared.exec();
		}
	}

	if (!newFoldersToAddInLibrary.isEmpty()) {
		this->doSearch(newFoldersToAddInLibrary);
	}

	// Process in reverse mode to clean cache: from database file and check if entry exists in database
	QStringList oldLocations;
	QSqlQuery cache = db->exec("SELECT * FROM filesystem");
	while (cache.next()) {
		QDir d(cache.record().value(0).toString());
		d.exists();
		QFileInfo fileInfo(cache.record().value(0).toString());
		// Remove folder in database because it couldn't be find in the filesystem
		if (!fileInfo.exists()) {
			db->exec("DELETE FROM filesystem WHERE path = \"" + fileInfo.absoluteFilePath() + "\"");
			oldLocations << fileInfo.absoluteFilePath();
		}
	}
	if (!oldLocations.isEmpty()) {
		db->rebuild(oldLocations, QStringList());
	}
}
