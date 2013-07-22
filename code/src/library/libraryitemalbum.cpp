#include "libraryitemalbum.h"

#include <QtDebug>

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
		QByteArray byteArray(dataLength, Qt::Uninitialized);
		in.readRawData(byteArray.data(), dataLength);
		setDisplayedName(QString(byteArray));
	}

	int year;
	in >> year;
	setYear(year);

	int trackCount;
	in >> trackCount;
	setData(trackCount, CHILD_COUNT);

	in >> dataLength;
	// If the path to the cover isn't null, there read it and build a new icon
	if (dataLength != 0xFFFFFFFF) {
		QByteArray byteArray(dataLength, Qt::Uninitialized);
		in.readRawData(byteArray.data(), dataLength);
		setCoverPath(QString(byteArray));
	}
}

/** Write data from this node to the output stream. */
void LibraryItemAlbum::write(QDataStream &out) const
{
	out << this->type();
	out << this->data(Qt::DisplayRole).toByteArray();
	out << this->year();
	out << this->rowCount();

	// It's useless to store the picture itself, it will be loaded when expanding items
	out << this->data(FILEPATH).toByteArray();
}

QString LibraryItemAlbum::coverPath() const
{
	return data(FILEPATH).toString();
}

void LibraryItemAlbum::setCoverPath(const QString &cover)
{
	setData(cover, FILEPATH);
}
