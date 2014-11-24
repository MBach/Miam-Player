#include "musicsearchengine.h"
#include "filehelper.h"
#include "settingsprivate.h"

#include <QDateTime>
#include <QDirIterator>
#include <QFileInfo>
#include <QThread>

#include <QtDebug>

MusicSearchEngine::MusicSearchEngine(QObject *parent) :
	QObject(parent)
{}

void MusicSearchEngine::doSearch(const QStringList &delta)
{
	QList<QDir> locations;
	QStringList pathsToSearch = delta.isEmpty() ? SettingsPrivate::instance()->musicLocations() : delta;
	foreach (QString musicPath, pathsToSearch) {
		QDir location(musicPath);
		location.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
		locations.append(location);
	}

	int entryCount = 0;
	// QDirIterator class is very fast to scan large directories
	foreach (QDir location, locations) {
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

	foreach (QDir location, locations) {
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
}
