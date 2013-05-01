#include "musicsearchengine.h"
#include "filehelper.h"
#include "settings.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QThread>

#include <QtDebug>

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
	QObject(parent)
{
	//QThread *worker = new QThread;
	//this->moveToThread(worker);
	//worker.start();
}

void MusicSearchEngine::doSearch()
{
	if (savedLocations.isEmpty()) {
		foreach (QVariant musicPath, Settings::getInstance()->musicLocations()) {
			QDir location(musicPath.toString());
			location.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden);
			savedLocations.append(location);
		}
	} else {
		foreach (QDir d, savedLocations) {
			qDebug() << d.absolutePath();
		}
	}
	int fileNumber = 0;
	for(int i=0; i<savedLocations.size(); i++) {

		// QDirIterator class is very fast to scan large directories
		QDirIterator it(savedLocations.at(i), QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			fileNumber++;
		}
	}

	int currentFile = 0;
	int percent = 1;
	bool aCoverWasFound = false;
	QString coverPath;
	/// FIXME : bug when iterating with |savedLocations| >1 : absolute file path are mixed with savedLocations
	for(int i=0; i<savedLocations.size(); i++) {
		QDirIterator it2(savedLocations.at(i), QDirIterator::Subdirectories);
		while (it2.hasNext()) {
			QFileInfo qFileInfo(it2.next());
			currentFile++;
			if (qFileInfo.suffix().toLower() == "jpg" || qFileInfo.suffix().toLower() == "png") {
				coverPath = qFileInfo.absoluteFilePath();
				aCoverWasFound = true;
			} else if (FileHelper::suffixes().contains(qFileInfo.suffix())) {
				//qDebug() << qFileInfo.absoluteFilePath().remove(savedLocations.at(i).absolutePath());
				emit scannedFile(i, qFileInfo.absoluteFilePath().remove(savedLocations.at(i).absolutePath()));
			} else { // unknown filetype, could be a directory, or anything else
				// if it's a directory, but excluding special folders, like "." and ".." then
				// we have to be sure that a we have found a cover before scanning a new directory
				if (qFileInfo.isDir() && (!qFileInfo.filePath().endsWith("..") && !qFileInfo.filePath().endsWith("."))) {
					if (aCoverWasFound) {
						emit scannedCover(coverPath);
						aCoverWasFound = false;
						//msleep(10);
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
	emit endSearch();
}
