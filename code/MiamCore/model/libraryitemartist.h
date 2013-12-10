#ifndef LIBRARYITEMARTIST_H
#define LIBRARYITEMARTIST_H

#include "libraryitem.h"

#include "miamcore_global.h"

class MIAMCORE_LIBRARY LibraryItemArtist : public LibraryItem
{
public:
	inline LibraryItemArtist(const QString &artist) : LibraryItem(artist) {}

	inline LibraryItemArtist() : LibraryItem() {}

	inline virtual ~LibraryItemArtist() {}

	/** Redefined for delegates (painting, etc). */
	//inline int type() const { return LibraryItem::Artist; }
};

#endif // LIBRARYITEMARTIST_H
