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
	: _file(NULL), _fileType(UNKNOWN)
{
	bool b = init(QDir::fromNativeSeparators(track.canonicalUrl().toLocalFile()));
	if (!b) {
		init(QDir::toNativeSeparators(track.canonicalUrl().toLocalFile()));
	}
}

FileHelper::FileHelper(const QString &filePath)
	: _file(NULL)
{
	qDebug() << Q_FUNC_INFO << filePath;
	if (!init(filePath)) {
		//init(filePath.toStdString().c_str());
		qDebug() << Q_FUNC_INFO << "couldn't init file, second chance!";
		bool b = init(filePath.toStdString().c_str());
		if (!b) {
			qDebug() << Q_FUNC_INFO << "couldn't init second chance :(";
		}
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
		_fileType = APE;
	} else if (suffix == "asf") {
		_file = new TagLib::ASF::File(fp);
		_fileType = ASF;
	} else if (suffix == "flac") {
		_file = new TagLib::FLAC::File(fp);
		_fileType = FLAC;
	} else if (suffix == "m4a" || suffix == "mp4") {
		_file = new TagLib::MP4::File(fp);
		_fileType = MP4;
	} else if (suffix == "mpc") {
		_file = new TagLib::MPC::File(fp);
		_fileType = MPC;
	} else if (suffix == "mp3") {
		_file = new TagLib::MPEG::File(fp);
		_fileType = MP3;
	} else if (suffix == "ogg" || suffix == "oga") {
		_file = new TagLib::Vorbis::File(fp);
		_fileType = OGG;
	} else if (suffix == "opus") {
		_file = new TagLib::Ogg::Opus::File(fp);
		_fileType = OGG;
	} else {
		_file = NULL;
		_fileType = UNKNOWN;
	}
	if (isValid()) {
		return true;
	} else if (_file != NULL) {
		delete _file;
		_file = NULL;
		_fileType = UNKNOWN;
	}
	return false;
}

FileHelper::~FileHelper()
{
	if (_file != NULL) {
		delete _file;
		_file = NULL;
	}
}

const QStringList FileHelper::suffixes(ExtensionType et, bool withPrefix)
{
	static QStringList standardSuffixes = QStringList() << "ape" << "asf" << "flac" << "m4a" << "mp4" << "mpc" << "mp3" << "oga" << "ogg" << "opus";
	static QStringList gameMusicEmuSuffixes = QStringList() << "ay" << "gbs" << "gym" << "hes" << "kss" << "nsf" << "nsfe" << "sap" << "spc" << "vgm" << "vgz";
	QStringList filters;
	if (et & Standard) {
		if (withPrefix) {
			for (QString filter : standardSuffixes) {
				filters.append("*." + filter);
			}
		} else {
			filters.append(standardSuffixes);
		}
	}
	if (et & GameMusicEmu) {
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
	case APE:
	case MPC:
		artAlb = this->extractGenericFeature("ALBUMARTIST");
		break;
	case OGG:
		artAlb = this->extractGenericFeature("ALBUMARTIST");
		if (artAlb.isEmpty()) {
			artAlb = this->extractVorbisFeature("ALBUM ARTIST");
		}
		break;
	case ASF:
		qDebug() << Q_FUNC_INFO << "Not yet implemented for ASF file";
		break;
	case FLAC:
		artAlb = this->extractFlacFeature("TPE2");
		break;
	case MP4:
		artAlb = this->extractGenericFeature("aART");
		break;
	case MP3:
		artAlb = this->extractMpegFeature("TPE2");
		break;
	}
	return artAlb.trimmed();
}

void FileHelper::setArtistAlbum(const QString &artistAlbum)
{
	switch (_fileType) {
	case FLAC: {
		this->setFlacAttribute("ALBUMARTIST", artistAlbum);
		break;
	}
	case MP4:{
		TagLib::MP4::File *mp4File = static_cast<TagLib::MP4::File*>(_file);
		TagLib::MP4::ItemListMap &items = mp4File->tag()->itemListMap();
		/*for (TagLib::Map<TagLib::String, TagLib::MP4::Item>::Iterator i = items.begin(); i != items.end(); i++) {
			if (i->first.toCString(true) == "aART") {

			}
		}*/
		items.insert("aART", artistAlbum.toStdString().data());
		break;
	}
	case MPC:
		//mpcFile = static_cast<MPC::File*>(f);
		qDebug() << Q_FUNC_INFO << "Not implemented for MPC";
		break;
	case MP3:{
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
	case OGG: {
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
	case APE:
	case MP4:
	case MPC:
	case OGG:
		strDiscNumber = this->extractGenericFeature("DISCNUMBER");
		break;
	case ASF:
		qDebug() << Q_FUNC_INFO << "Not yet implemented for ASF file";
		break;
	case FLAC:
		strDiscNumber = this->extractFlacFeature("TPOS");
		if (strDiscNumber.isEmpty()) {
			strDiscNumber = this->extractFlacFeature("DISCNUMBER");
		}
		break;
	case MP3:
		strDiscNumber = this->extractMpegFeature("TPOS");
		break;
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
	Cover *cover = NULL;
	switch (_fileType) {
	case MP3: {
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
	case FLAC: {
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

bool FileHelper::insert(QString key, const QVariant &value)
{
	// Standard tags
	TagLib::String v = value.toString().toStdString();

	/// XXX Create an enumeration somewhere

	if (key == "ALBUM") {
		_file->tag()->setAlbum(v);
	} else if (key == "ARTIST") {
		_file->tag()->setArtist(v);
	} else if (key == "COMMENT") {
		_file->tag()->setComment(v);
	} else if (key == "GENRE") {
		_file->tag()->setGenre(v);
	} else if (key == "TITLE") {
		_file->tag()->setTitle(v);
	} else if (key == "TRACKNUMBER") {
		_file->tag()->setTrack(value.toUInt());
	} else if (key == "YEAR") {
		_file->tag()->setYear(value.toUInt());
	} else if (key == "ARTISTALBUM") {
		this->setArtistAlbum(value.toString());
	} else if (key == "DISC") {
		this->setDiscNumber(value.toString());
	} else {
		return false;
	}
	return true;
}

/** Check if file has an inner picture. */
bool FileHelper::hasCover() const
{
	bool atLeastOnePicture = false;
	switch (_fileType) {
	case MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			// Look for picture frames only
			TagLib::ID3v2::FrameList listOfMp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			// It's possible to have more than one picture per file!
			if (!listOfMp3Frames.isEmpty()) {
				for (TagLib::ID3v2::FrameList::ConstIterator it = listOfMp3Frames.begin(); it != listOfMp3Frames.end() ; it++) {
					// Cast a Frame* to AttachedPictureFrame*
					TagLib::ID3v2::AttachedPictureFrame *pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
					//atLeastOnePicture = atLeastOnePicture || (pictureFrame != NULL && !pictureFrame->picture().isEmpty() && pictureFrame->type() != TagLib::ID3v2::AttachedPictureFrame::Other);
					atLeastOnePicture = atLeastOnePicture || (pictureFrame != NULL && !pictureFrame->picture().isEmpty());
				}
			}
		}
		break;
	}
	case FLAC: {
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
	case MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile && mpegFile->hasID3v2Tag()) {
			r = this->ratingForID3v2(mpegFile->ID3v2Tag());
		}
		break;
	}
	case FLAC: {
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
	case MP3: {
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
			if (cover != NULL) {
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
	case FLAC: {
		TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file);
		flacFile->removePictures();
		TagLib::FLAC::Picture *picture = new TagLib::FLAC::Picture;
		picture->setType(TagLib::FLAC::Picture::FrontCover);
		TagLib::ByteVector bv(cover->byteArray().data(), cover->byteArray().length());
		picture->setData(bv);
		flacFile->addPicture(picture);
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
	case FLAC: {
		this->setFlacAttribute("DISCNUMBER", disc);
		break;
	}
	case OGG: {
		TagLib::Ogg::XiphComment *xiphComment = static_cast<TagLib::Ogg::XiphComment*>(_file->tag());
		if (xiphComment) {
			xiphComment->addField("DISCNUMBER", disc.toStdString().data());
		} else {
			qDebug() << Q_FUNC_INFO << "Not implemented for this OGG file";
		}
		break;
	}
	case MP3: {
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
	default:
		qDebug() << Q_FUNC_INFO << "Not implemented for this file type";
		break;
	}
}

/** Set or remove any rating. */
void FileHelper::setRating(int rating)
{
	switch (_fileType) {
	case MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			this->setRatingForID3v2(rating, mpegFile->ID3v2Tag());
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << Q_FUNC_INFO << "Not implemented for ID3v1Tag";
		}
		break;
	}
	case FLAC: {
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
	return (_file != NULL && _file->isValid());
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
	if (_file && _file->tag() && _file->tag()->track() < INT_MAX) {
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
	if (_fileType == MP3) {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		// TagLib updates tags with the latest version (ID3v2.4)
		// We just want to save the file with the exact same version!
		if (mpegFile->hasID3v2Tag()) {
			return mpegFile->save(TagLib::MPEG::File::AllTags, false, mpegFile->ID3v2Tag()->header()->majorVersion());
		}
	} else if (_fileType != UNKNOWN) {
		return _file->save();
	}
	return false;
}

QString FileHelper::convertKeyToID3v2Key(QString key)
{
	/// TODO other relevant keys
	if (key.compare("ARTISTALBUM") == 0) {
		return "TPE2";
	} else if (key.compare("DISC") == 0) {
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
			TagLib::ID3v2::FrameList l = flacFile->ID3v2Tag()->frameListMap()[featureToExtract.toStdString().data()];
			if (!l.isEmpty()) {
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
			TagLib::ID3v2::FrameList l = tag->frameListMap()[attribute.data()];
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

void FileHelper::setRatingForID3v2(int rating, TagLib::ID3v2::Tag *tag)
{
	TagLib::ID3v2::FrameList l = tag->frameListMap()["POPM"];
	// If one wants to remove the existing rating
	if (rating == 0 && !l.isEmpty()) {
		tag->removeFrame(l.front());
	} else {
		TagLib::ID3v2::PopularimeterFrame *pf = NULL;
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
