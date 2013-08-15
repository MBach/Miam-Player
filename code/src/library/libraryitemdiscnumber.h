#ifndef LIBRARYITEMDISCNUMBER_H
#define LIBRARYITEMDISCNUMBER_H

#include "libraryitem.h"

class LibraryItemDiscNumber : public LibraryItem
{
public:
	explicit LibraryItemDiscNumber(int discNumber);

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Disc; }

	int discNumber() const { return data(DISC_NUMBER).toInt(); }
};

#endif // LIBRARYITEMDISCNUMBER_H
