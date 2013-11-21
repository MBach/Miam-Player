#ifndef LIBRARYITEMTRACK_H
#define LIBRARYITEMTRACK_H

#include "libraryitem.h"

class LibraryItemTrack : public LibraryItem
{
private:
	Q_ENUMS(CustomType)

public:
	explicit LibraryItemTrack() : LibraryItem() {}

	explicit LibraryItemTrack(const QString &track) : LibraryItem(track) {}

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Track; }

	inline void setAbsoluteFilePath(const QString &absPath, const QString &file) { setData(absPath, ABSOLUTE_PATH); setData(file, FILENAME); }
	inline QString absoluteFilePath() const { return data(ABSOLUTE_PATH).toString() + '/' + data(FILENAME).toString(); }

	inline void setDiscNumber(int track) { setData(track, DISC_NUMBER); }
	inline int discNumber() const { return data(DISC_NUMBER).toInt(); }

	inline void setTrackNumber(int track) { setData(track, TRACK_NUMBER); }
	inline int trackNumber() const { return data(TRACK_NUMBER).toInt(); }
};

#endif // LIBRARYITEMTRACK_H
