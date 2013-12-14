#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QtMultimedia/QMediaContent>
#include <QStringList>

#include "miamcore_global.h"

class Cover;

namespace TagLib {
	class File;
}

/**
 * @brief The FileHelper class is used to extract various but relevant fields in all types of tags (MP3, Flac, etc)
 */
class MIAMCORE_LIBRARY FileHelper
{
private:
	TagLib::File *_file;

	int fileType;

	static const QStringList suff;

	QString _absFilePath;

	Q_ENUMS(extension)

public:
	enum extension {
		APE		= 0,
		ASF		= 1,
		FLAC	= 2,
		MP4		= 4,
		MPC		= 5,
		MP3		= 6,
		OGG		= 7
	};

	FileHelper(const QMediaContent &track);

	FileHelper(const QString &filePath);

	virtual ~FileHelper();

	inline static QStringList suffixes() { return suff; }

	/** Field ArtistAlbum if exists (in a compilation for example). */
	QString artistAlbum() const;

	/** Extract the disc number. */
	int discNumber() const;

	/** Extract the inner picture if exists. */
	Cover* extractCover();

	bool insert(QString key, const QVariant &value);

	/** Convert the existing rating number into a smaller range from 1 to 5. */
	int rating() const;

	/** Sets the inner picture. */
	void setCover(Cover *cover);

	void setRating(int rating);

	/// Facade
	bool isValid() const;
	QString title() const;
	QString trackNumber() const;
	QString album() const;
	QString length() const;
	QString artist() const;
	QString year() const;
	QString genre() const;
	QString comment() const;
	bool save();

	QString absFilePath() const;

private:
	QString convertKeyToID3v2Key(QString key);

	QString extractFlacFeature(const QString &featureToExtract) const;

	QString extractMpegFeature(const QString &featureToExtract) const;
};

#endif // FILEHELPER_H
