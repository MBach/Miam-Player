#ifndef LIBRARYITEMDISCNUMBER_H
#define LIBRARYITEMDISCNUMBER_H

#include "libraryitem.h"

class MIAMCORE_LIBRARY LibraryItemDiscNumber : public LibraryItem
{
public:
	LibraryItemDiscNumber(int discNumber);

	inline LibraryItemDiscNumber() : LibraryItem() {}

	inline virtual ~LibraryItemDiscNumber() {}

	/** Redefined for delegates (painting, etc). */
	//inline int type() const { return LibraryItem::Disc; }

	inline int discNumber() const { return data(DISC_NUMBER).toInt(); }
};

#endif // LIBRARYITEMDISCNUMBER_H
