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
	QString _album, _artist, _artistAlbum, _disc, _iconPath, _length, _source, _title, _trackNumber, _url, _year;
	int _rating;

public:
	explicit TrackDAO(QObject *parent = 0);

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

	QString title() const;
	void setTitle(const QString &title);

	QString trackNumber() const;
	void setTrackNumber(const QString &trackNumber);

	QString url() const;
	void setUrl(const QString &url);

	QString year() const;
	void setYear(const QString &year);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(TrackDAO)

#endif // TRACKDAO_H
