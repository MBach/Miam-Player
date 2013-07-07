#include "libraryitem.h"
#include "librarymodel.h"
#include "starrating.h"
#include "settings.h"

#include <QPainter>

#include <QtDebug>

LibraryItem::LibraryItem(const QString &text, LibraryModel::MediaType mediaType, int type) :
	QStandardItem(text)
{
	this->setData(type, SUFFIX);
	this->setMediaType(mediaType);
	//if (text == "Benabar") {
	//	this->setData("aaaa", INTERNAL_NAME);
	//} else {
	this->setData(text, INTERNAL_NAME);
	//}
	setFont(Settings::getInstance()->font(Settings::LIBRARY));
}

LibraryItem::LibraryItem() :
	QStandardItem()
{
	this->setData(-1, SUFFIX);
}

/** Redefined for custom types (greater than Qt::UserRole). */
int LibraryItem::type() const
{
	return data(MEDIA_TYPE).toInt();
}

void LibraryItem::setDisplayedName(const char *name, int size)
{
	QByteArray byteArray(name, size);
	QVariant v(byteArray);
	setData(v.toString(), Qt::DisplayRole);
	setData(v.toString(), INTERNAL_NAME);
}

void LibraryItem::setFilePath(const QString &filePath)
{
	setData(QVariant(filePath), Qt::UserRole+1);
}

void LibraryItem::setFilePath(int musicLocationIndex, const QString &fileName)
{
	setData(QVariant(musicLocationIndex), IDX_TO_ABS_PATH);
	setData(QVariant(fileName), REL_PATH_TO_MEDIA);
}

/** Should only be used for tracks. */
void LibraryItem::setRating(int rating)
{
	setData(qVariantFromValue(StarRating(rating % 6)), STAR_RATING);
}

/** Should only be used for tracks. */
void LibraryItem::setTrackNumber(int trackNumber)
{
	setData(QVariant(trackNumber), TRACK_NUMBER);
}

/** Reads data from the input stream and fills informations in this new node. */
void LibraryItem::read(QDataStream &in)
{
	int type;
	in >> type;

	quint32 dataLength;
	char *s1;
	QVariant v;

	// See in the future if the 2 first conditions should be merged
	switch (type) {
	case LibraryModel::ALBUM:
		setMediaType(LibraryModel::ALBUM);
		in >> dataLength;

		// If we have saved an empty album, then the byte array is null (see Serializing Qt Data Types)
		if (dataLength != 0xFFFFFFFF) {
			char *s = new char[dataLength];
			in.readRawData(s, dataLength);
			setDisplayedName(s, dataLength);
			setData("abenabar", INTERNAL_NAME);
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
		break;

	case LibraryModel::ARTIST:
		setMediaType(LibraryModel::ARTIST);
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
		break;

	case LibraryModel::TRACK:
		setMediaType(LibraryModel::TRACK);

		in >> dataLength;
		setData(QVariant(dataLength), SUFFIX);

		in >> dataLength;

		// If we have saved an empty track, then the byte array is null (see Serializing Qt Data Types)
		if (dataLength != 0xFFFFFFFF) {
			char *s = new char[dataLength];
			in.readRawData(s, dataLength);
			setDisplayedName(s, dataLength);
			delete[] s;
		}
		in >> dataLength;
		setData(QVariant(dataLength), IDX_TO_ABS_PATH);

		in >> dataLength;
		s1 = new char[dataLength];
		in.readRawData(s1, dataLength);
		setData(QByteArray(s1, dataLength), REL_PATH_TO_MEDIA);
		delete[] s1;

		//in >> dataLength;
		//starRating = StarRating(dataLength);
		//v.setValue(starRating);
		//setData(v, STAR_RATING);

		in >> dataLength;
		setTrackNumber(dataLength);
		break;
	}
}

/** Write data from this node to the output stream. */
void LibraryItem::write(QDataStream &out) const
{
	int type = this->type();
	//StarRating starRating;

	switch (type) {
	case LibraryModel::ALBUM:
		out << type;
		out << data(Qt::DisplayRole).toByteArray();
		out << rowCount();

		// Save Absolute path + Relative path to picture, if exists
		// It's useless to store the picture itself, it will be loaded when expanding items
		out << data(REL_PATH_TO_MEDIA).toByteArray();
		if (!data(REL_PATH_TO_MEDIA).toString().isEmpty()) {
			out << data(IDX_TO_ABS_PATH).toInt();
		}
		break;

	case LibraryModel::ARTIST:
		out << type;
		out << data(Qt::DisplayRole).toByteArray();
		out << rowCount();
		break;

	case LibraryModel::TRACK:
		out << type;
		out << data(SUFFIX).toInt();
		out << data(Qt::DisplayRole).toByteArray();
		out << data(IDX_TO_ABS_PATH).toInt();
		out << data(REL_PATH_TO_MEDIA).toByteArray();
		//starRating = data(STAR_RATING).value<StarRating>();
		//out << starRating.starCount();
		out << trackNumber();
		break;
	}
}


void LibraryItem::setMediaType(LibraryModel::MediaType mediaType)
{
	setData(QVariant(mediaType), MEDIA_TYPE);
}
