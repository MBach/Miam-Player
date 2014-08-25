#ifndef REMOTETRACK_H
#define REMOTETRACK_H

#include <QIcon>
#include <QObject>
#include "../miamcore_global.h"

class MIAMCORE_LIBRARY RemoteTrack : public QObject
{
	Q_OBJECT
private:
	QString _album, _artist, _disc, _id, _length, _title, _trackNumber, _url, _year;
	QIcon _icon;
	int _rating;

public:
	explicit RemoteTrack(QObject *parent = 0);

	RemoteTrack(const RemoteTrack &remoteTrack);

	virtual ~RemoteTrack();

	QString album() const;
	void setAlbum(const QString &album);

	QString artist() const;
	void setArtist(const QString &artist);

	QString disc() const;
	void setDisc(const QString &disc);

	QIcon icon() const;
	void setIcon(const QIcon &icon);

	QString id() const;
	void setId(const QString &id);

	QString length() const;
	void setLength(const QString &length);

	int rating() const;
	void setRating(int rating);

	QString title() const;
	void setTitle(const QString &title);

	QString trackNumber() const;
	void setTrackNumber(const QString &trackNumber);

	QString url() const;
	void setUrl(const QString &url);

	QString year() const;
	void setYear(const QString &year);
};

#endif // REMOTETRACK_H
