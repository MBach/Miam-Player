#ifndef LIBRARYITEMALBUM_H
#define LIBRARYITEMALBUM_H

#include "libraryitem.h"
#include "persistentitem.h"

class LibraryItemAlbum : public LibraryItem
{
private:
	PersistentItem *_persistentItem;

public:
	explicit LibraryItemAlbum() : LibraryItem(), _persistentItem(NULL) {}

	explicit LibraryItemAlbum(const QString &album) : LibraryItem(album), _persistentItem(NULL) {}

	PersistentItem *persistentItem() const { return _persistentItem; }
	inline void setPersistentItem(PersistentItem *persistentItem) { _persistentItem = persistentItem; }

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Album; }

	int year() const { return data(YEAR).isValid() ? data(YEAR).toInt() : -1; }
	inline void setYear(int year) { setData(year, YEAR); }

	inline QString absolutePath() const { return data(ABSOLUTE_PATH).toString(); }
	inline void setAbsolutePath(const QString &absPath) { setData(absPath, ABSOLUTE_PATH); }

	inline QString coverFileName() const { return data(COVER_FILENAME).toString(); }
	inline void setCoverFileName(const QString &cover) { setData(cover, COVER_FILENAME); }
};

#endif // LIBRARYITEMALBUM_H
