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
#include <taglib/mpcfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
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
{
	bool b = init(QDir::fromNativeSeparators(track.canonicalUrl().toLocalFile()));
	if (!b) {
		init(QDir::toNativeSeparators(track.canonicalUrl().toLocalFile()));
	}
}

FileHelper::FileHelper(const QString &filePath)
{
	if (!init(filePath)) {
		//qDebug() << "second chance for" << filePath;
		init(filePath.toStdString().c_str());
		//init(filePath.toUtf8());
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
	TagLib::FileName fp(QFile::encodeName(QDir::toNativeSeparators(fileName)));
	//qDebug() << "FileHelper::init" << filePath;
	//QByteArray ba = QDir::toNativeSeparators(filePath).toLocal8Bit();
	//TagLib::FileName fp(ba.data());
	if (suffix == "ape") {
		_file = new TagLib::APE::File(fp);
		fileType = APE;
	} else if (suffix == "asf") {
		_file = new TagLib::ASF::File(fp);
		fileType = ASF;
	} else if (suffix == "flac") {
		_file = new TagLib::FLAC::File(fp);
		fileType = FLAC;
	} else if (suffix == "m4a") {
		_file = new TagLib::MP4::File(fp);
		fileType = MP4;
	} else if (suffix == "mpc") {
		_file = new TagLib::MPC::File(fp);
		fileType = MPC;
	} else if (suffix == "mp3") {
		_file = new TagLib::MPEG::File(fp);
		fileType = MP3;
	} else if (suffix == "ogg" || suffix == "oga") {
		_file = new TagLib::Vorbis::File(fp);
		fileType = OGG;
	} else {
		_file = NULL;
		fileType = UNKNOWN;
	}
	return (_file && _file->isValid());
}

FileHelper::~FileHelper()
{
	if (_file) {
		delete _file;
	}
}

const QStringList FileHelper::suffixes(bool withPrefix)
{
	QStringList suffixes = QStringList() << "ape" << "asf" << "flac" << "m4a" << "mpc" << "mp3" << "oga" << "ogg";
	if (withPrefix) {
		QStringList filters;
		foreach (QString filter, suffixes) {
			filters.append("*." + filter);
		}
		return filters;
	} else {
		return suffixes;
	}
}

/** Field ArtistAlbum if exists (in a compilation for example). */
QString FileHelper::artistAlbum() const
{
	QString artAlb = "";
	switch (fileType) {
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
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for ASF file";
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
	switch (fileType) {
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
		qDebug() << "MPC file";
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
			TagLib::ID3v2::TextIdentificationFrame *tif = new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("ARTISTALBUM"));
			tif->setText(artistAlbum.toStdString().data());
			tag->addFrame(tif);
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << "ID3v1Tag";
		}
		break;
	}
	default:
		qDebug() << "FileHelper::setArtistAlbum not implemented for this type of file";
		break;
	}
}

int FileHelper::discNumber(bool canBeZero) const
{
	QString strDiscNumber = "0";

	switch (fileType) {
	case APE:
	case MP4:
	case MPC:
	case OGG:
		strDiscNumber = this->extractGenericFeature("DISCNUMBER");
		break;
	case ASF:
		//asfFile = static_cast<ASF::File*>(f);
		qDebug() << "FileHelper::discNumber: Not yet implemented for ASF file";
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
	TagLib::MPEG::File *mpegFile = NULL;
	Cover *cover = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<TagLib::MPEG::File*>(_file);
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
			qDebug() << "FileHelper::extractCover: Not implemented for ID3v1Tag";
		}
		break;
	default:
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
		_file->tag()->setTrack(value.toInt());
	} else if (key == "YEAR") {
		_file->tag()->setYear(value.toInt());
	} else if (key == "ARTISTALBUM"){
		this->setArtistAlbum(value.toString());
	} else if (key == "DISC"){
		this->setDiscNumber(value.toString());
	} else {
		return false;
	}
	return true;
}

/** Check if file has an inner picture. */
bool FileHelper::hasCover() const
{
	TagLib::MPEG::File *mpegFile = NULL;
	bool atLeastOnePicture = false;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<TagLib::MPEG::File*>(_file);
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
		} //else if (mpegFile && mpegFile->hasID3v1Tag()) {
		//	qDebug() << "FileHelper::hasCover: Not implemented for ID3v1Tag";
		//}
		break;
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
	switch (fileType) {
	case MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			r = this->ratingForID3v2(mpegFile->ID3v2Tag());
		}
		break;
	}
	case FLAC: {
		TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file);
		if (flacFile->hasID3v2Tag()) {
			r = this->ratingForID3v2(flacFile->ID3v2Tag());
		} else if (flacFile->hasID3v1Tag()) {
			qDebug() << "FileHelper::rating: Not implemented (FLAC ID3v1)";
		} else if (flacFile->hasXiphComment()) {
			TagLib::StringList list = flacFile->xiphComment()->fieldListMap()["RATING"];
			if (!list.isEmpty()) {
				r = list.front().toInt();
			}
		}
		break;
	}
	default:
		// qDebug() << "FileHelper::rating: Not implemented";
		break;
	}
	return r;
}

/** Sets the inner picture. */
void FileHelper::setCover(Cover *cover)
{
	//qDebug() << "FileHelper::setCover, cover==NULL?" << (cover == NULL);
	TagLib::MPEG::File *mpegFile = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			// Look for picture frames only
			TagLib::ID3v2::FrameList mp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			if (!mp3Frames.isEmpty()) {
				for (TagLib::ID3v2::FrameList::Iterator it = mp3Frames.begin(); it != mp3Frames.end() ; it++) {
					// Removing a frame will invalidate any pointers on the list
					mpegFile->ID3v2Tag()->removeFrame(*it);
					qDebug() << "removing a frame";
					break;
				}
			}
			if (cover != NULL) {
				TagLib::ByteVector bv(cover->byteArray().data(), cover->byteArray().length());
				qDebug() << "cover->hasChanged()" << cover->hasChanged();
				TagLib::ID3v2::AttachedPictureFrame *pictureFrame = new TagLib::ID3v2::AttachedPictureFrame();
				//qDebug() << "cover.mimeType()" << QString(cover->mimeType());
				//qDebug() << "cover.mimeType2()" << QString::fromUtf8(cover->mimeType2().c_str());
				pictureFrame->setMimeType(cover->mimeType());
				pictureFrame->setPicture(bv);
				pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
				mpegFile->ID3v2Tag()->addFrame(pictureFrame);
				qDebug() << "adding a frame";
			}
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << "FileHelper::setCover: Not implemented for ID3v1Tag";
		}
		break;
	default:
		qDebug() << "FileHelper::setCover: Not implemented for" << fileType;
		break;
	}
}

/** Set or remove any disc number. */
void FileHelper::setDiscNumber(const QString &disc)
{
	switch (fileType) {
	case MP3: {
		TagLib::MPEG::File *mpegFile = mpegFile = static_cast<TagLib::MPEG::File*>(_file);
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
		qDebug() << "FileHelper::setDiscNumber: Not implemented for other file type than MP3";
		break;
	}
}

/** Set or remove any rating. */
void FileHelper::setRating(int rating)
{
	switch (fileType) {
	case MP3: {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		if (mpegFile->hasID3v2Tag()) {
			this->setRatingForID3v2(rating, mpegFile->ID3v2Tag());
		} else if (mpegFile->hasID3v1Tag()) {
			qDebug() << "FileHelper::rating: Not implemented for ID3v1Tag";
		}
		break;
	}
	case FLAC: {
		TagLib::FLAC::File *flacFile = static_cast<TagLib::FLAC::File*>(_file);
		if (flacFile->hasID3v2Tag()) {
			this->setRatingForID3v2(rating, flacFile->ID3v2Tag());
		} else if (flacFile->hasID3v1Tag()) {
			qDebug() << "hasID3v1Tag";
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
	if (_file && _file->tag()) {
		return QString("%1").arg(_file->tag()->track(), 2, 10, QChar('0')).toUpper();
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
	if (_file && _file->tag() && _file->tag()->year() > 0) {
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
	if (fileType == MP3) {
		TagLib::MPEG::File *mpegFile = static_cast<TagLib::MPEG::File*>(_file);
		// TagLib updates tags with the latest version (ID3v2.4)
		// We just want to save the file with the exact same version!
		if (mpegFile->hasID3v2Tag()) {
			return mpegFile->save(TagLib::MPEG::File::AllTags, false, mpegFile->ID3v2Tag()->header()->majorVersion());
		}
	}
	if (fileType != UNKNOWN) {
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
			qDebug() << "FileHelper::extractFlacFeature: Not yet implemented for ID3v1Tag FLAC file";
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
		} //else if (mpegFile->hasID3v1Tag()) {
			// qDebug() << "FileHelper::extractMpegFeature: Not yet implemented for ID3v1Tag MP3 file";
			// qDebug() << featureToExtract << _fileInfo.absoluteFilePath();
		//}
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
