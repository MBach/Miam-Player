#include "filehelper.h"
#include "cover.h"

#include <algorithm>
#include <map>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/apefile.h>
#include <taglib/apetag.h>
#include <taglib/asffile.h>
#include <taglib/flacfile.h>
#include <taglib/modfile.h>
#include <taglib/mpcfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/opusfile.h>
#include <taglib/vorbisfile.h>

#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>

#include <taglib/attachedpictureframe.h>
#include <taglib/popularimeterframe.h>
#include <taglib/tag.h>
#include <taglib/tlist.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

#include <taglib/tmap.h>
#include <taglib/tpropertymap.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4item.h>

#include <QtDebug>

FileHelper::FileHelper(const QMediaContent &track)
	: _file(nullptr)
	, _fileType(EXT_UNKNOWN)
	, _isValid(false)
{
	bool b = init(QDir::fromNativeSeparators(track.canonicalUrl().toLocalFile()));
	if (!b) {
		b = init(QDir::toNativeSeparators(track.canonicalUrl().toLocalFile()));
	}
	if (!b) {
		if (_file != nullptr) {
			delete _file;
			_file = nullptr;
		}
		_fileType = EXT_UNKNOWN;
	}
}

FileHelper::FileHelper(const QString &filePath)
	: _file(nullptr)
	, _fileType(EXT_UNKNOWN)
	, _isValid(false)
{
	bool b = init(filePath);
	if (!b) {
		b = init(filePath.toStdString().c_str());
	}
	if (!b) {
		delete _file;
		_file = nullptr;
		_fileType = EXT_UNKNOWN;
	}
}

std::string FileHelper::keyToStdString(Field f)
{
	switch (f) {
	case Field_Album:
		return "ALBUM";
	case Field_Artist:
		return "ARTIST";
	case Field_ArtistAlbum:
		return "ALBUMARTIST";
	case Field_Comment:
		return "COMMENT";
	case Field_Disc:
		return "ABSPATH";
	case Field_Genre:
		return "GENRE";
	case Field_Title:
		return "TITLE";
	case Field_Track:
		return "TRACKNUMBER";
	case Field_Year:
		return "DATE";
	default:
		return "";
	}
}

bool FileHelper::init(const QString &filePath)
{
	QString fileName;
	if (filePath.startsWith("file")) {
		fileName = filePath.mid(7);
	} else {
		fileName = filePath;
	}
	_fileInfo = QFileInfo(fileName);
	QString suffix = _fileInfo.suffix().toLower();
#ifdef _WIN32
	TagLib::FileName fp(QDir::toNativeSeparators(fileName).toStdWString().data());
#else
	TagLib::String s(QDir::toNativeSeparators(fileName).toUtf8().constData(), TagLib::String::UTF8);
	TagLib::FileName fp(s.toCString(true));
#endif
	if (suffix == "ape") {
		_file = new TagLib::APE::File(fp);
		_fileType = EXT_APE;
	} else if (suffix == "asf") {
		_file = new TagLib::ASF::File(fp);
		_fileType = EXT_ASF;
	} else if (suffix == "flac") {
		_file = new TagLib::FLAC::File(fp);
		_fileType = EXT_FLAC;
	} else if (suffix == "m4a" || suffix == "mp4") {
		_file = new TagLib::MP4::File(fp);
		_fileType = EXT_MP4;
	} else if (suffix == "mpc") {
		_file = new TagLib::MPC::File(fp);
		_fileType = EXT_MPC;
	} else if (suffix == "mp3") {
		_file = new TagLib::MPEG::File(fp);
		_fileType = EXT_MP3;
	} else if (suffix == "ogg" || suffix == "oga") {
		_file = new TagLib::Vorbis::File(fp);
		_fileType = EXT_OGG;
	} else if (suffix == "opus") {
		_file = new TagLib::Ogg::Opus::File(fp);
		_fileType = EXT_OGG;
	} else {
		_file = nullptr;
		_fileType = EXT_UNKNOWN;
	}
	if (_file != nullptr) {
		_isValid = true;
		return true;
	} else {
		delete _file;
		_file = nullptr;
		_fileType = EXT_UNKNOWN;
	}
	return false;
}

FileHelper::~FileHelper()
{
	if (_file != nullptr) {
		delete _file;
		_file = nullptr;
	}
}

const QStringList FileHelper::suffixes(ExtensionType et, bool withPrefix)
{
	static QStringList standardSuffixes = QStringList() << "ape" << "asf" << "flac" << "m4a" << "mp4" << "mpc" << "mp3" << "oga" << "ogg" << "opus";
	static QStringList gameMusicEmuSuffixes = QStringList() << "ay" << "gbs" << "gym" << "hes" << "kss" << "nsf" << "nsfe" << "sap" << "spc" << "vgm" << "vgz";
	QStringList filters;
	if (et & ET_Standard) {
		if (withPrefix) {
			for (QString filter : standardSuffixes) {
				filters.append("*." + filter);
			}
		} else {
			filters.append(standardSuffixes);
		}
	}
	if (et & ET_GameMusicEmu) {
		if (withPrefix) {
			for (QString filter : gameMusicEmuSuffixes) {
				filters.append("*." + filter);
			}
		} else {
			filters.append(gameMusicEmuSuffixes);
		}
	}
	return filters;
}

/** Field ArtistAlbum if exists (in a compilation for example). */
QString FileHelper::artistAlbum() const
{
	QString artAlb = "";
	if (!(_file && _file->tag())) {
		return artAlb;
	}
	switch (_fileType) {
	case EXT_APE:
	case EXT_MPC:
		artAlb = this->extractGenericFeature("ALBUMARTIST");
		break;
	case EXT_OGG:
		artAlb = this->extractGenericFeature("ALBUMARTIST");
		if (artAlb.isEmpty()) {
			artAlb = this->extractVorbisFeature("ALBUM ARTIST");
		}
		break;
	case EXT_ASF:
		qDebug() << Q_FUNC_INFO << "Not yet implemented for ASF file";
		break;
	case EXT_FLAC:
		artAlb = this->extractFlacFeature("ALBUMARTIST");
		break;
	case EXT_MP4: {
		artAlb = this->extractMp4Feature("aART");
		break;
	}
	case EXT_MP3:
		artAlb = this->extractMpegFeature("TPE2");
		break;
	}
	return artAlb.trimmed();
}

void FileHelper::setArtistAlbum(const QString &artistAlbum)
{
	switch (_fileType) {
	case EXT_FLAC: {
		this->setFlacAttribute("ALBUMARTIST", artistAlbum);
		break;
	}
	case EXT_MP4:{
		TagLib::StringList l;
		l.append(artistAlbum.toStdString().data());
		TagLib::MP4::Item item(l);
		this->setMp4Attribute("aART", item);
		break;
	}
	case EXT_MPC:
		//mpcFile = static_cast<MPC::File*>(f);
		qDebug() << Q_FUNC_INFO << "Not implemented for MPC";
		break;
	case EXT_MP3:{
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			TagLib::ID3v2::Tag *tag = mpegFile->ID3v2Tag();
			QString convertedKey = this->convertKeyToID3v2Key("ARTISTALBUM");
			TagLib::ID3v2::FrameList l = tag->frameListMap()[convertedKey.toStdString().data()];
			if (!l.isEmpty()) {
				tag->removeFrame(l.front());
			}
			TagLib::ID3v2::TextIdentificationFrame *tif = new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector(convertedKey.toStdString().data()));
			tif->setText(artistAlbum.toStdString().data());
			tag->addFrame(tif);
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented for ID3v1Tag";
		}
		break;
	}
	case EXT_OGG: {
		TagLib::Ogg::XiphComment *xiphComment = static_cast<TagLib::Ogg::XiphComment*>(_file->tag());
		if (xiphComment) {
			xiphComment->addField("ALBUMARTIST", artistAlbum.toStdString().data());
		} else {
			qDebug() << Q_FUNC_INFO << "Not implemented for this OGG file";
		}
		break;
	}
	default:
		qDebug() << Q_FUNC_INFO << "Not implemented for this type of file";
		break;
	}
}

int FileHelper::discNumber(bool canBeZero) const
{
	if (!(_file && _file->tag())) {
		return -1;
	}
	QString strDiscNumber = "0";

	switch (_fileType) {
	case EXT_APE:
	case EXT_MPC:
	case EXT_OGG:
		strDiscNumber = this->extractGenericFeature("DISCNUMBER");
		break;
	case EXT_ASF:
		qDebug() << Q_FUNC_INFO << "Not yet implemented for ASF file";
		break;
	case EXT_FLAC:
		strDiscNumber = this->extractFlacFeature("DISCNUMBER");
		break;
	case EXT_MP3:
		strDiscNumber = this->extractMpegFeature("TPOS");
		break;
	case EXT_MP4:
		strDiscNumber = this->extractGenericFeature("DISCNUMBER");
		break;
	default:
		qDebug() << Q_FUNC_INFO << "Not yet implemented for this file type" << _fileType;
	}
	int disc = -1;
	if (strDiscNumber.contains('/')) {
		disc = strDiscNumber.split('/').first().toInt();
	} else {
		disc = strDiscNumber.toInt();
		if (canBeZero && disc == 0) {
			disc = -1;
		}
	}
	return disc;
}

Cover* FileHelper::extractCover()
{
	Cover *cover = nullptr;
	switch (_fileType) {
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			// Look for picture frames only
			TagLib::ID3v2::FrameList listOfMp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			// It's possible to have more than one picture per file!
			if (!listOfMp3Frames.isEmpty()) {
				for (TagLib::ID3v2::FrameList::ConstIterator it = listOfMp3Frames.begin(); it != listOfMp3Frames.end() ; it++) {
					// Cast a Frame* to AttachedPictureFrame*
					TagLib::ID3v2::AttachedPictureFrame *pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
					if (pictureFrame) {
						// Performs a deep copy of the cover
						QByteArray b = QByteArray(pictureFrame->picture().data(), pictureFrame->picture().size());
						cover = new Cover(b, QString(pictureFrame->mimeType().toCString(true)));
					}
				}
			}
		} else if (mpegFile && mpegFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented for ID3v1Tag";
		}
		break;
	}
	case EXT_FLAC: {
		if (TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file)) {
			auto list = flacFile->pictureList();
			for (auto it = list.begin(); it != list.end() ; it++) {
				TagLib::FLAC::Picture *p = *it;
				if (p->type() == TagLib::FLAC::Picture::FrontCover) {
					// Performs a deep copy of the cover
					QByteArray b = QByteArray(p->data().data(), p->data().size());
					cover = new Cover(b, QString(p->mimeType().toCString(true)));
					break;
				}
			}
		}
		break;
	}
	default:
		qDebug() << Q_FUNC_INFO << "Not implemented for this file type" << _fileType << _file << _fileInfo.absoluteFilePath();
		break;
	}
	return cover;
}

bool FileHelper::insert(Field key, const QVariant &value)
{
	// Standard tags
	TagLib::String v = value.toString().toStdString();

	switch (key) {
	case Field_Album:
		_file->tag()->setAlbum(v);
		break;
	case Field_Artist:
		_file->tag()->setArtist(v);
		break;
	case Field_Comment:
		_file->tag()->setComment(v);
		break;
	case Field_Genre:
		_file->tag()->setGenre(v);
		break;
	case Field_Title:
		_file->tag()->setTitle(v);
		break;
	case Field_Track:
		_file->tag()->setTrack(value.toUInt());
		break;
	case Field_Year:
		_file->tag()->setYear(value.toUInt());
		break;
	case Field_ArtistAlbum:
		this->setArtistAlbum(value.toString());
		break;
	case Field_Disc:
		this->setDiscNumber(value.toString());
		break;
	default:
		return false;
	}
	return true;
}

/** Check if file has an inner picture. */
bool FileHelper::hasCover() const
{
	bool atLeastOnePicture = false;
	switch (_fileType) {
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			// Look for picture frames only
			TagLib::ID3v2::FrameList listOfMp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			// It's possible to have more than one picture per file!
			if (!listOfMp3Frames.isEmpty()) {
				for (TagLib::ID3v2::FrameList::ConstIterator it = listOfMp3Frames.begin(); it != listOfMp3Frames.end() ; it++) {
					// Cast a Frame* to AttachedPictureFrame*
					TagLib::ID3v2::AttachedPictureFrame *pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
					atLeastOnePicture = atLeastOnePicture || (pictureFrame != nullptr && !pictureFrame->picture().isEmpty());
				}
			}
		}
		break;
	}
	case EXT_FLAC: {
		if (TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file)) {
			atLeastOnePicture = !flacFile->pictureList().isEmpty();
		}
	}
	default:
		break;
	}
	return atLeastOnePicture;
}

/** Convert the existing rating number into a smaller range from 1 to 5. */
int FileHelper::rating() const
{
	int r = -1;

	/// TODO other types?
	switch (_fileType) {
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			r = this->ratingForID3v2(mpegFile->ID3v2Tag());
		}
		break;
	}
	case EXT_FLAC: {
		if (TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file)) {
			if (flacFile->hasID3v2Tag()) {
				r = this->ratingForID3v2(flacFile->ID3v2Tag());
			} else if (flacFile->hasID3v1Tag()) {
				qDebug() << Q_FUNC_INFO << "Not implemented (FLAC ID3v1)";
			} else if (flacFile->hasXiphComment()) {
				TagLib::StringList list = flacFile->xiphComment()->fieldListMap()["RATING"];
				if (!list.isEmpty()) {
					r = list.front().toInt();
				}
			}
		}
		break;
	}
	default:
		break;
	}
	return r;
}

/** Sets the inner picture. */
void FileHelper::setCover(Cover *cover)
{
	switch (_fileType) {
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			// Look for picture frames only
			TagLib::ID3v2::FrameList mp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			if (!mp3Frames.isEmpty()) {
				for (TagLib::ID3v2::FrameList::Iterator it = mp3Frames.begin(); it != mp3Frames.end() ; it++) {
					// Removing a frame will invalidate any pointers on the list
					mpegFile->ID3v2Tag()->removeFrame(*it);
					break;
				}
			}
			if (cover != nullptr) {
				TagLib::ByteVector bv(cover->byteArray().data(), cover->byteArray().length());
				TagLib::ID3v2::AttachedPictureFrame *pictureFrame = new TagLib::ID3v2::AttachedPictureFrame();
				pictureFrame->setMimeType(cover->mimeType());
				pictureFrame->setPicture(bv);
				pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
				mpegFile->ID3v2Tag()->addFrame(pictureFrame);
			}
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented for ID3v1Tag";
		}
		break;
	}
	case EXT_FLAC: {
		TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file);
		flacFile->removePictures();
		if (cover != nullptr) {
			TagLib::FLAC::Picture *picture = new TagLib::FLAC::Picture;
			picture->setType(TagLib::FLAC::Picture::FrontCover);
			TagLib::ByteVector bv(cover->byteArray().data(), cover->byteArray().length());
			picture->setData(bv);
			flacFile->addPicture(picture);
		}
		break;
	}
	default:
		qDebug() << Q_FUNC_INFO << "Not implemented for" << _fileType;
		break;
	}
}

/** Set or remove any disc number. */
void FileHelper::setDiscNumber(const QString &disc)
{
	switch (_fileType) {
	case EXT_FLAC: {
		this->setFlacAttribute("DISCNUMBER", disc);
		break;
	}
	case EXT_OGG: {
		TagLib::Ogg::XiphComment *xiphComment = static_cast<TagLib::Ogg::XiphComment*>(_file->tag());
		if (xiphComment) {
			xiphComment->addField("DISCNUMBER", disc.toStdString().data());
		} else {
			qDebug() << Q_FUNC_INFO << "Not implemented for this OGG file";
		}
		break;
	}
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			// Remove existing disc number if one has set an empty string
			if (disc.isEmpty()) {
				mpegFile->ID3v2Tag()->removeFrames(TagLib::ByteVector("TPOS"));
			} else {
				TagLib::ID3v2::TextIdentificationFrame *f = new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("TPOS"));
				f->setText(disc.toStdString());
				mpegFile->ID3v2Tag()->addFrame(f);
			}
		}
		break;
	}
	case EXT_MP4: {
		TagLib::MP4::Item item(disc.toUInt());
		this->setMp4Attribute("disk", item);
		break;
	}
	default:
		qDebug() << Q_FUNC_INFO << "Not implemented for this file type";
		break;
	}
}

/** Set or remove any rating. */
void FileHelper::setRating(int rating)
{
	switch (_fileType) {
	case EXT_MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			this->setRatingForID3v2(rating, mpegFile->ID3v2Tag());
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented for ID3v1Tag";
		}
		break;
	}
	case EXT_FLAC: {
		TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file);
		if (flacFile->hasID3v2Tag()) {
			this->setRatingForID3v2(rating, flacFile->ID3v2Tag());
		} else if (flacFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "hasID3v1Tag";
		} else if (flacFile->hasXiphComment()) {
			TagLib::Ogg::XiphComment *xiph = flacFile->xiphComment();
			if (rating == 0) {
				xiph->removeField("RATING");
			} else {
				xiph->addField("RATING", QString::number(rating).toStdString());
			}
		}
		break;
	}
	default:
		break;
	}
	this->save();
}

bool FileHelper::isValid() const
{
	/*if (_file) {
		qDebug() << Q_FUNC_INFO << _file->isOpen() << _file->isValid();
	}
	return (_file != nullptr && _file->isValid());*/
	return _isValid;
}

QString FileHelper::title() const
{
	if (_file && _file->tag()) {
		return QString(_file->tag()->title().toCString(true));
	} else {
		return QString("Error reading title");
	}
}

QString FileHelper::trackNumber() const
{
	if (_file && _file->tag() && _file->tag()->track() < UINT_MAX) {
		return QString("%1").arg(QString::number(_file->tag()->track()), 2, QChar('0')).toUpper();
	} else {
		return QString("00");
	}
}

QString FileHelper::album() const
{
	if (_file && _file->tag()) {
		return QString(_file->tag()->album().toCString(true)).trimmed();
	} else {
		return QString("Error reading album");
	}
}

QString FileHelper::length() const
{
	if (_file && _file->audioProperties()) {
		return QString::number(_file->audioProperties()->length());
	} else {
		return QString("0");
	}
}

QString FileHelper::artist() const
{
	if (_file && _file->tag()) {
		return QString(_file->tag()->artist().toCString(true)).trimmed();
	} else {
		return QString("Error reading artist");
	}
}

QString FileHelper::year() const
{
	if (_file && _file->tag() && _file->tag()->year() > 0 && _file->tag()->year() < INT_MAX) {
		return QString::number(_file->tag()->year());
	} else {
		return "";
	}
}

QString FileHelper::genre() const
{
	if (_file && _file->tag()) {
		return QString(_file->tag()->genre().toCString(true));
	} else {
		return "";
	}
}

QString FileHelper::comment() const
{
	if (_file && _file->tag()) {
		return QString(_file->tag()->comment().toCString(true));
	} else {
		return "";
	}
}

bool FileHelper::save()
{
	if (_fileType == EXT_MP3) {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		// TagLib updates tags with the latest version (ID3v2.4)
		// We just want to save the file with the exact same version!
		if (mpegFile->hasID3v2Tag()) {
			return mpegFile->save(TagLib::MPEG::File::AllTags, false, mpegFile->ID3v2Tag()->header()->majorVersion());
		}
	} else if (_fileType != EXT_UNKNOWN) {
		return _file->save();
	}
	return false;
}

QString FileHelper::convertKeyToID3v2Key(QString key) const
{
	/// TODO other relevant keys
	if (key.compare("ARTISTALBUM") == 0) {
		return "TPE2";
	} else if ((key.compare("DISC") == 0) || (key.compare("DISCNUMBER") == 0)) {
		return "TPOS";
	} else {
		return "";
	}
}

QString FileHelper::extractFlacFeature(const QString &featureToExtract) const
{
	QString feature;
	if (TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file)) {
		if (flacFile->ID3v2Tag()) {

			QString key = this->convertKeyToID3v2Key(featureToExtract);
			TagLib::ID3v2::FrameList l = flacFile->ID3v2Tag()->frameListMap()[key.toStdString().data()];
			// Fallback to the generic map in case we didn't find the matching key
			if (l.isEmpty()) {

				TagLib::StringList list = flacFile->properties()[featureToExtract.toStdString()];
				if (!list.isEmpty()) {
					feature = list.front().toCString(true);
				}

			} else {
				feature = QString(l.front()->toString().toCString(true));
			}
		} else if (flacFile->ID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not yet implemented for ID3v1Tag FLAC file";
		} else if (flacFile->xiphComment()) {
			const TagLib::Ogg::FieldListMap map = flacFile->xiphComment()->fieldListMap();
			if (!map[featureToExtract.toStdString().data()].isEmpty()) {
				feature = QString(map[featureToExtract.toStdString().data()].front().toCString(true));
			}
		}
	}
	return feature;
}

QString FileHelper::extractGenericFeature(const QString &featureToExtract) const
{
	QString feature;
	TagLib::PropertyMap p = _file->properties();
	if (p.contains(featureToExtract.toStdString().data())) {
		TagLib::StringList list = p[featureToExtract.toStdString().data()];
		if (!list.isEmpty()) {
			feature = list.front().toCString(true);
		}
	}
	return feature;
}
QString FileHelper::extractMp4Feature(const QString &featureToExtract) const
{
	QString feature;
	if (TagLib::MP4::File *mp4File = static_cast<TagLib::MP4::File*>(_file)) {
		if (mp4File->tag()) {
			TagLib::MP4::ItemListMap &items = mp4File->tag()->itemListMap();
			if (items.contains(featureToExtract.toStdString().data())) {
				TagLib::MP4::Item item = items[featureToExtract.toStdString().data()];
				TagLib::StringList list = item.toStringList();
				if (list.size() > 0) {
					feature = list[0].toCString(true);
				}
				/*for (uint i = 0; i < list.size(); i++) {
					TagLib::String s = list[i];
					qDebug() << Q_FUNC_INFO << s.toCString(true);
				}*/
			}
			/*for (auto it = items.begin(); it != items.end(); ++it) {
				qDebug() << Q_FUNC_INFO << QString(it->first.toCString(false));
			}*/
		}
	}
	return feature;
}
QString FileHelper::extractMpegFeature(const QString &featureToExtract) const
{
	QString feature;
	if (TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file)) {
		if (mpegFile->hasID3v2Tag()) {
			TagLib::ID3v2::FrameList l = mpegFile->ID3v2Tag()->frameListMap()[featureToExtract.toStdString().data()];
			if (!l.isEmpty()) {
				feature = QString(l.front()->toString().toCString(true));
			}
		}
	}
	return feature;
}

QString FileHelper::extractVorbisFeature(const QString &featureToExtract) const
{
	QString feature;
	if (TagLib::Vorbis::File *vorbisFile = static_cast<TagLib::Vorbis::File*>(_file)) {
		if (vorbisFile->tag()) {
			const TagLib::Ogg::FieldListMap map = vorbisFile->tag()->fieldListMap();
			if (vorbisFile->tag()->properties().contains(featureToExtract.toStdString().data())) {
				feature = QString(map[featureToExtract.toStdString().data()].front().toCString(true));
			}
		}
	}
	return feature;
}

int FileHelper::ratingForID3v2(TagLib::ID3v2::Tag *tag) const
{
	int r = -1;
	TagLib::ID3v2::FrameList l = tag->frameListMap()["POPM"];
	if (l.isEmpty()) {
		return r;
	}
	if (TagLib::ID3v2::PopularimeterFrame *pf = static_cast<TagLib::ID3v2::PopularimeterFrame*>(l.front())) {
		switch (pf->rating()) {
		case 1:
			r = 1;
			break;
		case 64:
			r = 2;
			break;
		case 128:
			r = 3;
			break;
		case 196:
			r = 4;
			break;
		case 255:
			r = 5;
			break;
		}
	}
	return r;
}

void FileHelper::setFlacAttribute(const std::string &attribute, const QString &value)
{
	if (TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file)) {
		if (flacFile->hasID3v2Tag()) {
			TagLib::ID3v2::Tag *tag = flacFile->ID3v2Tag();
			QString key = this->convertKeyToID3v2Key(attribute.data());
			TagLib::ID3v2::FrameList l = tag->frameListMap()[key.toStdString().data()];
			if (!l.isEmpty()) {
				tag->removeFrame(l.front());
			}
			TagLib::ID3v2::TextIdentificationFrame *tif = new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("ARTISTALBUM"));
			tif->setText(value.toStdString().data());
			tag->addFrame(tif);
		} else if (flacFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented (FLAC ID3v1)";
		} else if (flacFile->hasXiphComment()) {
			TagLib::Ogg::XiphComment *xiph = flacFile->xiphComment();
			if (value.isEmpty()) {
				xiph->removeField(attribute.data());
			} else {
				xiph->addField(attribute.data(), value.toStdString().data());
			}
		}
	}
}

void FileHelper::setMp4Attribute(const std::string &attribute, const TagLib::MP4::Item &value)
{
	if (TagLib::MP4::File *mp4File = static_cast<TagLib::MP4::File*>(_file)) {
		if (mp4File->tag()) {
			TagLib::MP4::ItemListMap &items = mp4File->tag()->itemListMap();
			items.insert(attribute, value);
		}
	}
}

void FileHelper::setRatingForID3v2(int rating, TagLib::ID3v2::Tag *tag)
{
	TagLib::ID3v2::FrameList l = tag->frameListMap()["POPM"];
	// If one wants to remove the existing rating
	if (rating == 0 && !l.isEmpty()) {
		tag->removeFrame(l.front());
	} else {
		TagLib::ID3v2::PopularimeterFrame *pf = nullptr;
		if (l.isEmpty()) {
			pf = new TagLib::ID3v2::PopularimeterFrame();
			tag->addFrame(pf);
		} else {
			pf = static_cast<TagLib::ID3v2::PopularimeterFrame*>(l.front());
		}
		switch (rating) {
		case 1:
			pf->setRating(1);
			break;
		case 2:
			pf->setRating(64);
			break;
		case 3:
			pf->setRating(128);
			break;
		case 4:
			pf->setRating(196);
			break;
		case 5:
			pf->setRating(255);
			break;
		}
	}
}
