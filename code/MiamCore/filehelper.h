#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QtMultimedia/QMediaContent>
#include <QStringList>

#include "miamcore_global.h"

#include <QFileInfo>

class Cover;

namespace TagLib {
	class File;

	namespace ID3v2 {
		class Tag;
	}
}

/**
 * \brief The FileHelper class is used to extract various but relevant fields in all types of tags (MP3, Flac, etc)
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY FileHelper
{
private:
	TagLib::File *_file;

	int _fileType;

	QFileInfo _fileInfo;

	Q_ENUMS(extension)
	Q_ENUMS(ExtensionType)

public:
	enum extension {
		UNKNOWN = -1,
		APE		= 0,
		ASF		= 1,
		FLAC	= 2,
		MP4		= 4,
		MPC		= 5,
		MP3		= 6,
		OGG		= 7
	};

	enum ExtensionType {
		Standard		= 0x1,
		GameMusicEmu	= 0x2,
		All				= Standard | GameMusicEmu
	};

	enum TagKey {
		Artist
	};

	FileHelper(const QMediaContent &track);

	FileHelper(const QString &filePath);

private:
	bool init(const QString &filePath);

public:
	virtual ~FileHelper();

	static const QStringList suffixes(ExtensionType et = Standard, bool withPrefix = false);

	/** Field ArtistAlbum if exists (in a compilation for example). */
	QString artistAlbum() const;
	void setArtistAlbum(const QString &artistAlbum);

	/** Extract field disc number. */
	int discNumber(bool canBeZero = false) const;

	/** Extract the inner picture if exists. */
	Cover* extractCover();

	bool insert(QString key, const QVariant &value);

	/** Check if file has an inner picture. */
	bool hasCover() const;

	/** Convert the existing rating number into a smaller range from 1 to 5. */
	int rating() const;

	/** Sets the inner picture. */
	void setCover(Cover *cover);

	/** Set or remove any disc number. */
	void setDiscNumber(const QString &disc);

	/** Set or remove any rating. */
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
	inline QFileInfo fileInfo() const { return _fileInfo; }

	inline TagLib::File *file() { return _file; }

private:
	QString convertKeyToID3v2Key(QString key);

	QString extractFlacFeature(const QString &featureToExtract) const;
	QString extractGenericFeature(const QString &featureToExtract) const;
	QString extractMpegFeature(const QString &featureToExtract) const;
	QString extractVorbisFeature(const QString &featureToExtract) const;

	int ratingForID3v2(TagLib::ID3v2::Tag *tag) const;
	void setFlacAttribute(const std::string &attribute, const QString &value);
	void setRatingForID3v2(int rating, TagLib::ID3v2::Tag *tag);
};

#endif // FILEHELPER_H
