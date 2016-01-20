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
	QString _artist, _artistNormalized, _disc, _cover, _length, _source, _uri, _year;
	uint _artistID;

public:
	explicit AlbumDAO(QObject *parent = nullptr);

	AlbumDAO(const AlbumDAO &remoteAlbum);

	AlbumDAO& operator=(const AlbumDAO& other);

	virtual ~AlbumDAO();

	uint artistID() const;
	void setArtistID(const uint &artistID);

	QString artist() const;
	void setArtist(const QString &artist);

	QString artistNormalized() const;
	void setArtistNormalized(const QString &artistNormalized);

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

	virtual uint hash() const override;
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(AlbumDAO)

#endif // ALBUMDAO_H
