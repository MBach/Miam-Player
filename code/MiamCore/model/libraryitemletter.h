#ifndef LIBRARYITEMLETTER_H
#define LIBRARYITEMLETTER_H

#include "libraryitem.h"

class MIAMCORE_LIBRARY LibraryItemLetter : public LibraryItem
{
public:
	inline LibraryItemLetter(const QString &letter) : LibraryItem(letter) {}

	inline LibraryItemLetter() : LibraryItem() {}

	inline virtual ~LibraryItemLetter() {}

	/** Redefined for delegates (painting, etc). */
	//inline int type() const { return LibraryItem::Letter; }
};

#endif // LIBRARYITEMLETTER_H
