#ifndef COVER_H
#define COVER_H

#include <QString>

class Cover
{
private:
	const char* _mimeType;
	const char* _format;
	QString _album;
	QByteArray _data;

public:
	Cover();

	Cover(const QByteArray &byteArray, const char* mimeType);

	const char* mimeType() const { return _mimeType; }

	QByteArray byteArray() const { return _data; }

	void setAlbum(const QString &album) { _album = album; }

	QString album() const { return _album; }

	const char* format() { return _format; }
};

#endif // COVER_H
