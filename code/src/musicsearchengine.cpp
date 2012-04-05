#include "musicsearchengine.h"

#include "settings.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QMetaType>

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
    QThread(parent)
{
	qRegisterMetaType<QFileInfo>("QFileInfo");
}

void MusicSearchEngine::run()
{
	QList<QVariant> musicPaths = Settings::getInstance()->musicLocations();
	int fileNumber = 0;
	for(int i=0; i<musicPaths.size(); i++) {

		// QDirIterator class is very fast to scan large directories
		QDirIterator it(musicPaths.at(i).toString(), QDir::AllDirs | QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			fileNumber++;
		}
	}

	int currentFile = 0;
	int percent = 1;
	bool isFolder = true;
	bool aCoverWasFound = false;
	QString coverPath;
	for(int i=0; i<musicPaths.size(); i++) {
		QDirIterator it2(musicPaths.at(i).toString(), QDir::AllDirs | QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
		while (it2.hasNext()) {
			QFileInfo qFileInfo(it2.next());
			currentFile++;
			if (qFileInfo.suffix().toLower() == "jpg" || qFileInfo.suffix().toLower() == "png") {
				isFolder = false;
				coverPath = qFileInfo.absoluteFilePath();
				aCoverWasFound = true;
			} else if (qFileInfo.suffix().toLower() == "mp3") {
				isFolder = false;
				emit scannedFile(i, qFileInfo.absoluteFilePath().remove(musicPaths.at(i).toString()));
			} else { // unknown filetype, could be a directory, or anything else
				// if it's a directory, but excluding special folders, like "." and ".." then
				// we have to be sure that a we have found a cover before scanning a new directory
				if (qFileInfo.isDir() && (!qFileInfo.filePath().endsWith("..") && !qFileInfo.filePath().endsWith("."))) {
					if (aCoverWasFound) {
						emit scannedCover(coverPath);
						aCoverWasFound = false;
					}
					isFolder = true;
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
}
