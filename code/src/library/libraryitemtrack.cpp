#include "libraryitemtrack.h"

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
		char *s = new char[dataLength];
		in.readRawData(s, dataLength);
		setDisplayedName(s, dataLength);
		delete[] s;
	}
	in >> dataLength;
	setData(QVariant(dataLength), IDX_TO_ABS_PATH);

	in >> dataLength;
	char *s1 = new char[dataLength];
	in.readRawData(s1, dataLength);
	setData(QByteArray(s1, dataLength), REL_PATH_TO_MEDIA);
	delete[] s1;

	in >> dataLength;
	setTrackNumber(dataLength);
}

/** Write data from this node to the output stream. */
void LibraryItemTrack::write(QDataStream &out) const
{
	out << type();
	out << data(SUFFIX).toInt();
	out << data(Qt::DisplayRole).toByteArray();
	out << data(IDX_TO_ABS_PATH).toInt();
	out << data(REL_PATH_TO_MEDIA).toByteArray();
	out << trackNumber();
}
