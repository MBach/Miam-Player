#ifndef LIBRARYITEMLETTER_H
#define LIBRARYITEMLETTER_H

#include "libraryitem.h"

class LibraryItemLetter : public LibraryItem
{
public:
	explicit LibraryItemLetter(const QString &letter) : LibraryItem(letter) {}

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Letter; }
};

#endif // LIBRARYITEMLETTER_H
