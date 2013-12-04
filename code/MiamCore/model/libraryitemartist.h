#ifndef LIBRARYITEMARTIST_H
#define LIBRARYITEMARTIST_H

#include "libraryitem.h"

class MIAMCORE_LIBRARY LibraryItemArtist : public LibraryItem
{
public:
	explicit LibraryItemArtist(const QString &artist) : LibraryItem(artist) {}

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Artist; }
};

#endif // LIBRARYITEMARTIST_H
