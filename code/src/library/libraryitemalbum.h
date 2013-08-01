#ifndef LIBRARYITEMALBUM_H
#define LIBRARYITEMALBUM_H

#include "libraryitem.h"

class LibraryItemAlbum : public LibraryItem
{
public:
	explicit LibraryItemAlbum() : LibraryItem() {}

	explicit LibraryItemAlbum(const QString &album);

	void setYear(int year);
	int year() const;

	/** Redefined. */
	inline int type() const { return LibraryItem::Album; }

	QString coverPath() const;
	void setCoverPath(const QString &cover);
};

#endif // LIBRARYITEMALBUM_H
