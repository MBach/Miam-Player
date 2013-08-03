#include "libraryitemtrack.h"

#include "settings.h"

#include <QtDebug>

LibraryItemTrack::LibraryItemTrack(const QString &track, int suffix)
	: LibraryItem(track)
{
	this->setData(suffix, SUFFIX);
}

/** Reads data from the input stream and fills informations in this new node. */
void LibraryItemTrack::read(QDataStream &in)
{
	QList<CustomType> properties;
	properties << FILEPATH << ARTIST << ARTISTALBUM << ALBUM << TITLE << TRACK_NUMBER << YEAR;

	quint32 dataLength;
	foreach (CustomType property, properties) {
		in >> dataLength;
		// If we have saved an empty property, then the byte array is null (see Serializing Qt Data Types)
		if (dataLength != 0xFFFFFFFF) {
			QByteArray byteArray(dataLength, Qt::Uninitialized);
			in.readRawData(byteArray.data(), dataLength);
			setData(QString(byteArray), property);
		}
	}
}

/** Write data from this node to the output stream. */
void LibraryItemTrack::write(QDataStream &out) const
{
	out << data(FILEPATH).toByteArray();
	out << data(ARTIST).toByteArray();
	out << data(ARTISTALBUM).toByteArray();
	out << data(ALBUM).toByteArray();
	out << data(Qt::DisplayRole).toByteArray();
	out << data(TRACK_NUMBER).toByteArray();
	out << data(YEAR).toByteArray();
}
