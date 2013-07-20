#include "libraryitemalbum.h"

LibraryItemAlbum::LibraryItemAlbum(const QString &album)
	: LibraryItem(album)
{
}

void LibraryItemAlbum::setYear(int year)
{
	this->setData(year, Qt::UserRole + 5);
}

int LibraryItemAlbum::year() const
{
	if (data(Qt::UserRole + 5).isValid()) {
		return data(Qt::UserRole + 5).toInt();
	} else {
		return -1;
	}
}

/** Reads data from the input stream and fills informations in this new node. */
void LibraryItemAlbum::read(QDataStream &in)
{
	quint32 dataLength;
	in >> dataLength;

	// If we have saved an empty album, then the byte array is null (see Serializing Qt Data Types)
	if (dataLength != 0xFFFFFFFF) {
		char *s = new char[dataLength];
		in.readRawData(s, dataLength);
		setDisplayedName(s, dataLength);
		delete[] s;
	}

	int trackCount;
	in >> trackCount;
	setData(trackCount, CHILD_COUNT);

	in >> dataLength;
	// If the path to the cover isn't null, there read it and build a new icon
	if (dataLength != 0xFFFFFFFF) {

		// Relative path to image album
		char *s = new char[dataLength];
		in.readRawData(s, dataLength);
		setData(QByteArray(s, dataLength), REL_PATH_TO_MEDIA);
		delete[] s;

		// Reference to Absolute path
		in >> dataLength;
		setData(QVariant(dataLength), IDX_TO_ABS_PATH);
	}
}

/** Write data from this node to the output stream. */
void LibraryItemAlbum::write(QDataStream &out) const
{
	out << this->type();
	out << data(Qt::DisplayRole).toByteArray();
	out << rowCount();

	// Save Absolute path + Relative path to picture, if exists
	// It's useless to store the picture itself, it will be loaded when expanding items
	out << data(REL_PATH_TO_MEDIA).toByteArray();
	if (!data(REL_PATH_TO_MEDIA).toString().isEmpty()) {
		out << data(IDX_TO_ABS_PATH).toInt();
	}
}
