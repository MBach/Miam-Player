#include "libraryitemtrack.h"

#include "settings.h"

LibraryItemTrack::LibraryItemTrack(const QString &track, int suffix)
	: LibraryItem(track)
{
	this->setData(suffix, SUFFIX);
}

/** Reads data from the input stream and fills informations in this new node. */
void LibraryItemTrack::read(QDataStream &in)
{
	quint32 dataLength;
	in >> dataLength;
	setData(QVariant(dataLength), SUFFIX);

	// If we have saved an empty track, then the byte array is null (see Serializing Qt Data Types)
	in >> dataLength;
	if (dataLength != 0xFFFFFFFF) {
		QByteArray byteArray(dataLength, Qt::Uninitialized);
		in.readRawData(byteArray.data(), dataLength);
		setDisplayedName(QString(byteArray));
	}

	in >> dataLength;	
	QByteArray byteArray(dataLength, Qt::Uninitialized);
	in.readRawData(byteArray.data(), dataLength);
	setData(QString(byteArray), FILEPATH);

	in >> dataLength;
	setTrackNumber(dataLength);
}

/** Write data from this node to the output stream. */
void LibraryItemTrack::write(QDataStream &out) const
{
	out << type();
	out << data(SUFFIX).toInt();
	out << data(Qt::DisplayRole).toByteArray();
	out << data(FILEPATH).toByteArray();
	out << trackNumber();
}

QString LibraryItemTrack::filePath() const
{
	return data(FILEPATH).toString();
}

void LibraryItemTrack::setFilePath(const QString &absFilePath)
{
	setData(absFilePath, FILEPATH);
}
