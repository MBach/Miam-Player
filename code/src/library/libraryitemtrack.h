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

	inline QString absoluteFilePath() const { return data(ABSOLUTE_PATH).toString() + '/' + data(FILENAME).toString(); }

	inline void setAbsolutePath(const QString &absolutePath) { setData(absolutePath, ABSOLUTE_PATH);}
	inline QString absolutePath() const { return data(ABSOLUTE_PATH).toString(); }

	inline void setDiscNumber(int track) { setData(track, DISC_NUMBER); }
	inline int discNumber() const { return data(DISC_NUMBER).toInt(); }

	inline void setFileName(const QString &fileName) { setData(fileName, FILENAME); }
	inline QString fileName() const { return data(FILENAME).toString(); }

	inline void setTrackNumber(int track) { setData(track, TRACK_NUMBER); }
	inline int trackNumber() const { return data(TRACK_NUMBER).toInt(); }
};

#endif // LIBRARYITEMTRACK_H
