#ifndef LIBRARYITEMARTIST_H
#define LIBRARYITEMARTIST_H

#include "libraryitem.h"

class LibraryItemArtist : public LibraryItem
{
public:
	explicit LibraryItemArtist() : LibraryItem() {}

	explicit LibraryItemArtist(const QString &artist);

	/** Reads data from the input stream and fills informations in this new node. */
	virtual void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	virtual void write(QDataStream &out) const;

	/** Redefined. */
	inline int type() const { return LibraryItem::Artist; }
};

#endif // LIBRARYITEMARTIST_H
