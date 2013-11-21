#include "musicsearchengine.h"
#include "filehelper.h"
#include "settings.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QThread>

#include <QtDebug>

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
	QObject(parent)
{}

void MusicSearchEngine::doSearch()
{
	QList<QDir> savedLocations;
	foreach (QString musicPath, Settings::getInstance()->musicLocations()) {
		QDir location(musicPath);
		location.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden);
		savedLocations.append(location);
	}

	int fileNumber = 0;
	// QDirIterator class is very fast to scan large directories
	foreach (QDir location, savedLocations) {
		QDirIterator it(location, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			fileNumber++;
		}
	}

	int currentFile = 0;
	int percent = 1;
	bool aCoverWasFound = false;
	QString coverPath;

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
				//qDebug() << "doSearch" << qFileInfo.absoluteFilePath();
				emit scannedFiled(qFileInfo.absoluteFilePath());
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
