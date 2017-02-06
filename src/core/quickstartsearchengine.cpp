#include "quickstartsearchengine.h"

#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QThread>

#include "filehelper.h"

QuickStartSearchEngine::QuickStartSearchEngine(QObject *parent)
	: QObject(parent)
{}

void QuickStartSearchEngine::doSearch()
{
	QString userMusicPath = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
	QDir musicDir(userMusicPath);

	// For every subfolder in the user's music path, a quick test on multimedia files is done
	for (QFileInfo fileInfo : musicDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {

		QDirIterator it(fileInfo.absoluteFilePath(), FileHelper::suffixes(FileHelper::ET_Standard, true), QDir::Files, QDirIterator::Subdirectories);
		int musicFiles = 0;
		while (it.hasNext()) {
			it.next();
			musicFiles++;
		}
		emit folderScanned(fileInfo, musicFiles);
	}
	emit quickSearchHasEnded();
}
