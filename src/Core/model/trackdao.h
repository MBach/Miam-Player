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
	QString _album, _artist, _artistAlbum, _disc, _length, _source, _trackNumber, _uri, _year;
	int _rating;

public:
	explicit TrackDAO(QObject *parent = nullptr);

	TrackDAO(const TrackDAO &other);

	TrackDAO& operator=(const TrackDAO& other);

	virtual ~TrackDAO();

	QString album() const;
	void setAlbum(const QString &album);

	QString artist() const;
	void setArtist(const QString &artist);

	QString artistAlbum() const;
	void setArtistAlbum(const QString &artistAlbum);

	QString disc() const;
	void setDisc(const QString &disc);

	QString length() const;
	void setLength(const QString &length);

	int rating() const;
	void setRating(int rating);

	QString source() const;
	void setSource(const QString &source);

	QString trackNumber(bool twoDigits = false) const;
	void setTrackNumber(const QString &trackNumber);

	QString uri() const;
	void setUri(const QString &uri);

	QString year() const;
	void setYear(const QString &year);

	virtual uint hash() const;
};

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator<<(QDataStream &out, const TrackDAO &track)
{
	out << track.uri();
	out << track.artistAlbum();
	out << track.album();
	out << track.title();
	out << track.trackNumber();

	return out;
}

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator>>(QDataStream &in, TrackDAO &track)
{
	QString uri, aa, a, ti, tn;
	in >> uri;
	track.setUri(uri);
	in >> aa;
	track.setArtistAlbum(aa);
	in >> a;
	track.setAlbum(a);
	in >> ti;
	track.setTitle(ti);
	in >> tn;
	track.setTrackNumber(tn);
	return in;
}

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(TrackDAO)

#endif // TRACKDAO_H
