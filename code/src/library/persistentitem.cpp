#include "persistentitem.h"

#include <QtDebug>

PersistentItem::PersistentItem(const LibraryItemTrack *track)
	: LibraryItemTrack(track->text())
{
	this->setAbsolutePath(track->absolutePath());
	this->setDiscNumber(track->discNumber());
	this->setFileName(track->fileName());
	this->setTrackNumber(track->trackNumber());
}

/** Reads data from the input stream and fills informations in this new node. */
void PersistentItem::read(QDataStream &in)
{
	static QList<CustomType> properties = (QList<CustomType>() << ABSOLUTE_PATH << FILENAME << COVER_FILENAME << ARTIST << ARTISTALBUM << ALBUM << TITLE << TRACK_NUMBER << DISC_NUMBER << YEAR);
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
	static QList<CustomType> properties = (QList<CustomType>() << ABSOLUTE_PATH << FILENAME << COVER_FILENAME << ARTIST << ARTISTALBUM << ALBUM << TITLE << TRACK_NUMBER << DISC_NUMBER << YEAR);
	foreach (CustomType property, properties) {
		/// FIXME
		//if (property == COVER_FILENAME) {
		//	qDebug() << album() << coverFileName();
		//}
		out << data(property).toByteArray();
	}
}
