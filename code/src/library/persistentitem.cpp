#include "persistentitem.h"

#include <QtDebug>

PersistentItem::PersistentItem(LibraryItemTrack *track)
	: LibraryItemTrack(track->text())
{
	this->setAbsolutePath(track->absolutePath());
	this->setFileName(track->fileName());
	this->setTrackNumber(track->trackNumber());
}

/** Reads data from the input stream and fills informations in this new node. */
void PersistentItem::read(QDataStream &in)
{
	QList<CustomType> properties;
	properties << ABSOLUTE_PATH << FILENAME << COVER_FILENAME << ARTIST << ARTISTALBUM << ALBUM << TITLE << TRACK_NUMBER << YEAR;

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
void PersistentItem::write(QDataStream &out) const
{
	QList<CustomType> properties;
	properties << ABSOLUTE_PATH << FILENAME << COVER_FILENAME << ARTIST << ARTISTALBUM << ALBUM << TITLE << TRACK_NUMBER << YEAR;
	foreach (CustomType property, properties) {
		out << data(property).toByteArray();
	}
}
