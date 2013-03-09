#ifndef FILEHELPER_H
#define FILEHELPER_H

#include "tageditor/cover.h"

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

	Q_ENUMS(extension)

public:
	FileHelper(TagLib::FileRef &fileRef, QVariant v);

	enum extension {
		APE		= 0,
		ASF		= 1,
		FLAC	= 2,
		MP4		= 4,
		MPC		= 5,
		MP3		= 6,
		OGG		= 7
	};

	FileHelper(const QString &filePath);

	TagLib::String artistAlbum() const;

	Cover* extractCover();

	inline TagLib::File *file() const { return f; }

	bool insert(QString key, const QVariant &value);

	inline bool save() { return f->save(); }

	void setCover(Cover *cover);

	inline static QStringList suffixes() { return suff; }

	inline int type() const { return fileType; }

private:
	QString convertKeyToID3v2Key(QString key);
};

#endif // FILEHELPER_H
