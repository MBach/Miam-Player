#ifndef ALBUMDAO_H
#define ALBUMDAO_H

#include "genericdao.h"

class MIAMCORE_LIBRARY AlbumDAO : public GenericDAO
{
	Q_OBJECT
private:
	QString _artist, _disc, _iconPath, _length, _source, _title, _uri, _year;

public:
	explicit AlbumDAO(QObject *parent = 0);

	AlbumDAO(const AlbumDAO &remoteAlbum);

	virtual ~AlbumDAO();

	QString artist() const;
	void setArtist(const QString &artist);

	QString disc() const;
	void setDisc(const QString &disc);

	QString iconPath() const;
	void setIconPath(const QString &iconPath);

	QString length() const;
	void setLength(const QString &length);

	QString source() const;
	void setSource(const QString &source);

	QString title() const;
	void setTitle(const QString &title);

	QString uri() const;
	void setUri(const QString &uri);

	QString year() const;
	void setYear(const QString &year);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(AlbumDAO)

#endif // ALBUMDAO_H
