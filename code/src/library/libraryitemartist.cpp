#include "libraryitemartist.h"

LibraryItemArtist::LibraryItemArtist(const QString &artist)
	: LibraryItem(artist)
{
}

/** Reads data from the input stream and fills informations in this new node. */
void LibraryItemArtist::read(QDataStream &in)
{
	quint32 dataLength;
	in >> dataLength;

	// If we have saved an empty artist, then the byte array is null (see Serializing Qt Data Types)
	if (dataLength != 0xFFFFFFFF) {
		char *s = new char[dataLength];
		in.readRawData(s, dataLength);
		setDisplayedName(s, dataLength);
		delete[] s;
	}

	int albumCount;
	in >> albumCount;
	setData(albumCount, CHILD_COUNT);
}

/** Write data from this node to the output stream. */
void LibraryItemArtist::write(QDataStream &out) const
{
	out << type();
	out << data(Qt::DisplayRole).toByteArray();
	out << rowCount();
}
