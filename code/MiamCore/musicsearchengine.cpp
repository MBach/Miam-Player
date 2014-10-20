#include "musicsearchengine.h"
#include "filehelper.h"
#include "settingsprivate.h"

#include <QDateTime>
#include <QDirIterator>
#include <QFileInfo>
#include <QThread>

#include <QtDebug>

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
	QObject(parent), _watcher(new QFileSystemWatcher(this))
{
	/*_watcher->addPaths(Settings::getInstance()->musicLocations());
	connect(_watcher, &QFileSystemWatcher::directoryChanged, [=](const QString &path) {
		qDebug() << "directory has changed:" << path;
		QDirIterator it(path, QDirIterator::NoIteratorFlags);
		while (it.hasNext()) {
			it.next();
			qDebug() << "d:" << it.fileInfo().absoluteFilePath() << it.fileInfo().lastModified();
		}
	});
	connect(_watcher, &QFileSystemWatcher::fileChanged, [=](const QString &path) {
		qDebug() << "file has changed:" << path;
	});*/
}

void MusicSearchEngine::doSearch()
{
	//qDebug() << Q_FUNC_INFO;
	QList<QDir> savedLocations;
	foreach (QString musicPath, SettingsPrivate::getInstance()->musicLocations()) {
		QDir location(musicPath);
		location.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
		savedLocations.append(location);
	}

	int entryCount = 0;
	// QDirIterator class is very fast to scan large directories
	foreach (QDir location, savedLocations) {
		QDirIterator it(location.absolutePath(), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
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

	foreach (QDir location, savedLocations) {
		QDirIterator it(location.absolutePath(), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString entry = it.next();
			// qDebug() << "entry" << entry;
			QFileInfo qFileInfo(entry);
			currentEntry++;

			// Directory has changed: we can discard cover
			if (qFileInfo.isDir()) {
				// qDebug() << "directory changed";
				if (!coverPath.isEmpty() && !lastFileScannedNextToCover.isEmpty()) {
					// qDebug() << "cover found (sent now!)" << coverPath << lastFileScannedNextToCover;
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
					// qDebug() << "cover found" << coverPath;
					coverPath.clear();
				} else if (isNewDirectory) {
					coverPath = qFileInfo.absoluteFilePath();
					// qDebug() << "cover found (not sent)" << coverPath;
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
}
