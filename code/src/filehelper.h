#ifndef FILEHELPER_H
#define FILEHELPER_H

#include "tageditor/cover.h"

#include <QStringList>
#include <QVariant>

#include <taglib.h>
#include <fileref.h>

/**
 * @brief The FileHelper class is used to extract various but relevant fields in all types of tags (MP3, Flac, etc)
 */
class FileHelper
{
private:
	TagLib::File *f;

	int fileType;

	static const QStringList suff;

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

	~FileHelper() {
		delete f;
	}

	inline static QStringList suffixes() { return suff; }

	inline TagLib::File *file() const { return f; }

	/** Field ArtistAlbum if exists (in a compilation for example). */
	QString artistAlbum() const;

	int discNumber() const;

	/** Extract the inner picture if exists. */
	Cover* extractCover();

	bool insert(QString key, const QVariant &value);

	/** Convert the existing rating number into a smaller range from 1 to 5. */
	int rating() const;

	/** Sets the inner picture. */
	void setCover(Cover *cover);

private:
	QString convertKeyToID3v2Key(QString key);

	QString extractFlacFeature(const QString &featureToExtract) const;

	QString extractMpegFeature(const QString &featureToExtract) const;
};

#endif // FILEHELPER_H
