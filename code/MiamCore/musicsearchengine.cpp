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

	int fileNumber = 0;
	// QDirIterator class is very fast to scan large directories
	//QMultiHash<QDir, QDir> dirs;
	foreach (QDir location, savedLocations) {
		QDirIterator it(location, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			//if (it.fileInfo().isDir()) {
				//QDir d = it.fileInfo().dir();
				//QFileInfoList fil = d.entryInfoList(QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
				//qDebug() << "observing" << it.filePath();
				//dirs.insert();
			//}
			fileNumber++;
		}
	}

	int currentFile = 0;
	int percent = 1;
	bool aCoverWasFound = false;
	QString coverPath;

	///XXX: improve with setNameFilters (*.mp3 != entry in FileHelper::suffixes())
	foreach (QDir location, savedLocations) {
		QDirIterator it(location, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString path = it.next();
			QFileInfo qFileInfo(path);
			currentFile++;
			if (qFileInfo.suffix().toLower() == "jpg" || qFileInfo.suffix().toLower() == "png") {
				coverPath = qFileInfo.absoluteFilePath();
				aCoverWasFound = true;
			} else if (FileHelper::suffixes().contains(qFileInfo.suffix())) {
				emit scannedFile(qFileInfo.absoluteFilePath());
			} else { // unknown filetype, could be a directory, or anything else
				// if it's a directory, but excluding special folders, like "." and ".." then
				// we have to be sure that a we have found a cover before scanning a new directory
				if (qFileInfo.isDir() && (!qFileInfo.filePath().endsWith("..") && !qFileInfo.filePath().endsWith("."))) {
					if (aCoverWasFound) {
						emit scannedCover(coverPath);
						aCoverWasFound = false;
					}
				}
			}
			if (currentFile * 100 / fileNumber > percent) {
				percent = currentFile * 100 / fileNumber;
				emit progressChanged(percent);
			}
		}
		// After the while loop, it's possible to have one more cover to send
		if (aCoverWasFound) {
			emit scannedCover(coverPath);
			aCoverWasFound = false;
		}
	}
	emit searchHasEnded();
}
