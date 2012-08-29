#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QStringList>
#include <QVariant>

#include <taglib.h>
#include <fileref.h>

class FileHelper
{
private:
	TagLib::File *f;

	int fileType;

	static QStringList suff;

public:
	FileHelper(TagLib::FileRef &fileRef, QVariant v);

	FileHelper(const QString &filePath);

	TagLib::String artistAlbum() const;

	bool insert(QString key, QString value);

	bool save();

	inline TagLib::File *file() const { return f; }

	inline int type() const { return fileType; }

	inline static QStringList suffixes() { return suff; }

private:
	QString convertKeyToID3v2Key(QString key);
};

#endif // FILEHELPER_H
