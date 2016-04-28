#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QtMultimedia/QMediaContent>
#include <QStringList>

#include "miamcore_global.h"

#include <QFileInfo>

/// Forward declaration
class Cover;

/// Forward declaration
namespace TagLib {
	class File;

	namespace ID3v2 {
		class Tag;
	}

	namespace MP4 {
		class Item;
	}
}

/**
 * \brief		The FileHelper class is used to extract various but relevant fields in all types of tags (MP3, Flac, etc).
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY FileHelper
{
private:
	TagLib::File *_file;

	int _fileType;
	bool _isValid;

	QFileInfo _fileInfo;

	Q_ENUMS(Extension)
	Q_ENUMS(ExtensionType)
	Q_ENUMS(Field)

public:
	enum Extension {
		EXT_UNKNOWN = -1,
		EXT_APE		= 0,
		EXT_ASF		= 1,
		EXT_FLAC	= 2,
		EXT_MP4		= 4,
		EXT_MPC		= 5,
		EXT_MP3		= 6,
		EXT_OGG		= 7
	};

	enum ExtensionType {
		ET_Standard		= 0x1,
		ET_GameMusicEmu	= 0x2,
		ET_All			= ET_Standard | ET_GameMusicEmu
	};

	enum TagKey {
		Artist
	};

	enum Field {
		Field_AbsPath		= 1,
		Field_Album			= 2,
		Field_Artist		= 3,
		Field_ArtistAlbum	= 4,
		Field_Comment		= 5,
		Field_Cover			= 6,
		Field_Disc			= 7,
		Field_FileName		= 8,
		Field_Genre			= 9,
		Field_Title			= 10,
		Field_Track			= 11,
		Field_Year			= 12
	};

	explicit FileHelper(const QMediaContent &track);

	explicit FileHelper(const QString &filePath);

	static std::string keyToStdString(Field f);

private:
	bool init(const QString &filePath);

public:
	virtual ~FileHelper();

	static const QStringList suffixes(ExtensionType et = ET_Standard, bool withPrefix = false);

	/** Field ArtistAlbum if exists (in a compilation for example). */
	QString artistAlbum() const;
	void setArtistAlbum(const QString &artistAlbum);

	/** Extract field disc number. */
	int discNumber(bool canBeZero = false) const;

	/** Extract the inner picture if exists. */
	Cover* extractCover();

	bool insert(Field key, const QVariant &value);

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
	QString convertKeyToID3v2Key(QString key) const;

	QString extractFlacFeature(const QString &featureToExtract) const;
	QString extractGenericFeature(const QString &featureToExtract) const;
	QString extractMp4Feature(const QString &featureToExtract) const;
	QString extractMpegFeature(const QString &featureToExtract) const;
	QString extractVorbisFeature(const QString &featureToExtract) const;

	int ratingForID3v2(TagLib::ID3v2::Tag *tag) const;
	void setFlacAttribute(const std::string &attribute, const QString &value);
	void setMp4Attribute(const std::string &attribute, const TagLib::MP4::Item &value);
	void setRatingForID3v2(int rating, TagLib::ID3v2::Tag *tag);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(FileHelper::Field)

#endif // FILEHELPER_H
