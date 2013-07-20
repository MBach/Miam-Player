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

	/** Reads data from the input stream and fills informations in this new node. */
	virtual void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	virtual void write(QDataStream &out) const;

	/** Redefined. */
	inline int type() const { return LibraryItem::Album; }
};

#endif // LIBRARYITEMALBUM_H
