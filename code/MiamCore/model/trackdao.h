#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QIcon>
#include "genericdao.h"

/**
 * \brief		The TrackDAO class is a simple wrapper which contains basic informations about a file.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY TrackDAO : public GenericDAO
{
	Q_OBJECT
private:
	QString _album, _artist, _artistAlbum, _disc, _iconPath, _length, _source, _trackNumber, _uri, _year;
	int _rating;

public:
	explicit TrackDAO(QObject *parentNode = 0);

	TrackDAO(const TrackDAO &remoteTrack);

	virtual ~TrackDAO();

	QString album() const;
	void setAlbum(const QString &album);

	QString artist() const;
	void setArtist(const QString &artist);

	QString artistAlbum() const;
	void setArtistAlbum(const QString &artistAlbum);

	QString disc() const;
	void setDisc(const QString &disc);

	QString iconPath() const;
	void setIconPath(const QString &iconPath);

	QString length() const;
	void setLength(const QString &length);

	int rating() const;
	void setRating(int rating);

	QString source() const;
	void setSource(const QString &source);

	QString trackNumber() const;
	void setTrackNumber(const QString &trackNumber);

	QString uri() const;
	void setUri(const QString &uri);

	QString year() const;
	void setYear(const QString &year);
};

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator<<(QDataStream &out, const TrackDAO &track)
{
	out << track.uri();
	return out;
}

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator>>(QDataStream &in, TrackDAO &track)
{
	QString f;
	in >> f;
	track.setUri(f);
	return in;
}

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(TrackDAO)

#endif // TRACKDAO_H
