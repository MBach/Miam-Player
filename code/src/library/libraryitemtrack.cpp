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
	setData(QVariant(dataLength), IDX_TO_ABS_PATH);

	in >> dataLength;	
	QByteArray byteArray(dataLength, Qt::Uninitialized);
	in.readRawData(byteArray.data(), dataLength);
	setData(QString(byteArray), REL_PATH_TO_MEDIA);

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

QString LibraryItemTrack::filePath() const
{
	int index = data(IDX_TO_ABS_PATH).toInt();
	QString absPath = Settings::getInstance()->musicLocations().at(index).toString();
	QString relPath = data(REL_PATH_TO_MEDIA).toString();
	return absPath + relPath;
}

void LibraryItemTrack::setFilePath(int musicLocationIndex, const QString &fileName)
{
	setData(QVariant(musicLocationIndex), IDX_TO_ABS_PATH);
	setData(QVariant(fileName), REL_PATH_TO_MEDIA);
}
