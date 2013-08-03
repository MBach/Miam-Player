#ifndef LIBRARYITEMLETTER_H
#define LIBRARYITEMLETTER_H

#include "libraryitem.h"

class LibraryItemLetter : public LibraryItem
{
public:
	explicit LibraryItemLetter(const QString &letter);

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Letter; }

	/** Reads data from the input stream and fills informations in this new node. */
	inline void read(QDataStream &) {}

	/** Write data from this node to the output stream. */
	inline void write(QDataStream &) const {}
};

#endif // LIBRARYITEMLETTER_H
