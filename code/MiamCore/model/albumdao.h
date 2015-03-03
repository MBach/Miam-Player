#ifndef ALBUMDAO_H
#define ALBUMDAO_H

#include "genericdao.h"

/**
 * \brief		The AlbumDAO class is a simple wrapper.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY AlbumDAO : public GenericDAO
{
	Q_OBJECT
private:
	QString _artist, _disc, _cover, _length, _source, _uri, _year;

public:
	explicit AlbumDAO(QObject *parentNode = 0);

	AlbumDAO(const AlbumDAO &remoteAlbum);

	AlbumDAO& operator=(const AlbumDAO& other);

	virtual ~AlbumDAO();

	QString artist() const;
	void setArtist(const QString &artist);

	QString disc() const;
	void setDisc(const QString &disc);

	QString cover() const;
	void setCover(const QString &cover);

	QString length() const;
	void setLength(const QString &length);

	QString source() const;
	void setSource(const QString &source);

	QString uri() const;
	void setUri(const QString &uri);

	QString year() const;
	void setYear(const QString &year);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(AlbumDAO)

#endif // ALBUMDAO_H
