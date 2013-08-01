#include "libraryitemtrack.h"

#include "settings.h"

LibraryItemTrack::LibraryItemTrack(const QString &track, int suffix)
	: LibraryItem(track)
{
	this->setData(suffix, SUFFIX);
}

QString LibraryItemTrack::filePath() const
{
	return data(FILEPATH).toString();
}

void LibraryItemTrack::setFilePath(const QString &absFilePath)
{
	setData(absFilePath, FILEPATH);
}
