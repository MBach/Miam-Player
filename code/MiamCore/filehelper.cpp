#include "filehelper.h"

#include <QFileInfo>

// Taglib headers
#include <apefile.h>
#include <asffile.h>
#include <flacfile.h>
#include <mpcfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <vorbisfile.h>

#include <id3v2tag.h>
#include <id3v2frame.h>

#include <attachedpictureframe.h>
#include <popularimeterframe.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>

#include "cover.h"

#include <QDateTime>
#include <QImage>
#include <QtDebug>

const QStringList FileHelper::suff = QStringList() << "ape" << "asf" << "flac" << "m4a" << "mpc" << "mp3" << "oga" << "ogg";

#include <3rdparty/taglib/taglib.h>
#include <3rdparty/taglib/fileref.h>

using namespace TagLib;

FileHelper::FileHelper(const QMediaContent &track)
	: FileHelper(track.canonicalUrl().toLocalFile())
{

}

FileHelper::FileHelper(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	QString suffix = fileInfo.suffix().toLower();
	const char *fp = QFile::encodeName(filePath).constData();
	if (suffix == "ape") {
		_file = new APE::File(fp);
		fileType = APE;
	} else if (suffix == "asf") {
		_file = new ASF::File(fp);
		fileType = ASF;
	} else if (suffix == "flac") {
		_file = new FLAC::File(fp);
		fileType = FLAC;
	} else if (suffix == "m4a") {
		_file = new MP4::File(fp);
		fileType = MP4;
	} else if (suffix == "mpc") {
		_file = new MPC::File(fp);
		fileType = MPC;
	} else if (suffix == "mp3") {
		_file = new MPEG::File(fp);
		fileType = MP3;
	} else if (suffix == "ogg" || suffix == "oga") {
		_file = new Vorbis::File(fp);
		fileType = OGG;
	} else {
		_file = NULL;
		fileType = -1;
	}
}

FileHelper::~FileHelper()
{
	delete _file;
}

/** Field ArtistAlbum if exists (in a compilation for example). */
QString FileHelper::artistAlbum() const
{
	QString artAlb = "";
	/// TODO
	//APE::File *apeFile = NULL;
	//ASF::File *asfFile = NULL;
	//FLAC::File *flacFile = NULL;
	//MPC::File *mpcFile = NULL;
	//MP4::File *mp4File = NULL;
	//MPEG::File *mpegFile = NULL;
	//Ogg::File *oggFile = NULL;

	switch (fileType) {
	case APE:
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for APE file";
		break;
	case ASF:
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for ASF file";
		break;
	case FLAC:
		artAlb = this->extractFlacFeature("TPE2");
		break;
	case MP4:
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for MP4 file";
		break;
	case MPC:
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for MPC file";
		break;
	case MP3:
		artAlb = this->extractMpegFeature("TPE2");
		break;
	case OGG:
		qDebug() << "FileHelper::artistAlbum: Not yet implemented for OGG file";
		break;
	}
	return artAlb.trimmed();
}

int FileHelper::discNumber() const
{
	/// TODO
	//APE::File *apeFile = NULL;
	//ASF::File *asfFile = NULL;
	//FLAC::File *flacFile = NULL;
	//MPC::File *mpcFile = NULL;
	//MP4::File *mp4File = NULL;
	//MPEG::File *mpegFile = NULL;
	//Ogg::File *oggFile = NULL;

	QString strDiscNumber = "1";

	switch (fileType) {
	case APE:
		//apeFile = static_cast<APE::File*>(f);
		qDebug() << "FileHelper::discNumber: Not yet implemented for APE file";
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
	case MP4:
		//mp4File = static_cast<MP4::File*>(f);
		qDebug() << "FileHelper::discNumber: Not yet implemented for MP4 file";
		break;
	case MPC:
		//mpcFile = static_cast<MPC::File*>(f);
		qDebug() << "FileHelper::discNumber: Not yet implemented for MPC file";
		break;
	case MP3:
		strDiscNumber = this->extractMpegFeature("TPOS");
		break;
	case OGG:
		//oggFile = static_cast<Ogg::File*>(f);
		break;
	}
	int disc = 1;
	if (strDiscNumber.contains('/')) {
		disc = strDiscNumber.split('/').first().toInt();
	} else {
		disc = strDiscNumber.toInt();
	}
	return disc;
}

Cover* FileHelper::extractCover()
{
	MPEG::File *mpegFile = NULL;
	Cover *cover = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(_file);
		if (mpegFile->ID3v2Tag()) {
			// Look for picture frames only
			ID3v2::FrameList listOfMp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			// It's possible to have more than one picture per file!
			if (!listOfMp3Frames.isEmpty()) {
				for (ID3v2::FrameList::ConstIterator it = listOfMp3Frames.begin(); it != listOfMp3Frames.end() ; it++) {
					// Cast a Frame* to AttachedPictureFrame*
					ID3v2::AttachedPictureFrame *pictureFrame = static_cast<ID3v2::AttachedPictureFrame*>(*it);
					// Performs a deep copy of the cover
					QByteArray b = QByteArray(pictureFrame->picture().data(), pictureFrame->picture().size());
					cover = new Cover(b, QString(pictureFrame->mimeType().toCString(true)));
				}
			}
		} else if (mpegFile->ID3v1Tag()) {
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
	String v = value.toString().toStdString();

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
	} else {
		// Other non generic tags, like Artist Album
		//APE::File *apeFile = NULL;
		//ASF::File *asfFile = NULL;
		//FLAC::File *flacFile = NULL;
		//MP4::File *mp4File = NULL;
		//MPC::File *mpcFile = NULL;
		MPEG::File *mpegFile = NULL;

		switch (fileType) {
		case APE:
			//apeFile = static_cast<APE::File*>(f);
			qDebug() << "APE file";
			break;
		case ASF:
			//asfFile = static_cast<ASF::File*>(f);
			qDebug() << "ASF file";
			break;
		case FLAC:
			//flacFile = static_cast<FLAC::File*>(f);
			qDebug() << "FLAC file";
			break;
		case MP4:
			//mp4File = static_cast<MP4::File*>(f);
			qDebug() << "MP4 file";
			break;
		case MPC:
			//mpcFile = static_cast<MPC::File*>(f);
			qDebug() << "MPC file";
			break;
		case MP3:
			mpegFile = static_cast<MPEG::File*>(_file);
			if (mpegFile->ID3v2Tag()) {
				ID3v2::Tag *tag = mpegFile->ID3v2Tag();
				if (tag) {
					QString convertedKey = this->convertKeyToID3v2Key(key);
					ID3v2::FrameList l = tag->frameListMap()[convertedKey.toStdString().data()];
					if (!l.isEmpty()) {
						tag->removeFrame(l.front());
					}
					ID3v2::TextIdentificationFrame *tif = new ID3v2::TextIdentificationFrame(ByteVector(convertedKey.toStdString().data()));
					tif->setText(value.toString().toStdString().data());
					tag->addFrame(tif);
				}
			} else if (mpegFile->ID3v1Tag()) {
				qDebug() << "ID3v1Tag";
			}
			break;
		case OGG:
			qDebug() << "OGG file";
			break;
		}
	}
	return true;
}

/** Convert the existing rating number into a smaller range from 1 to 5. */
int FileHelper::rating() const
{
	MPEG::File *mpegFile = NULL;
	int r = -1;

	/// TODO other types?
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(_file);
		if (mpegFile->ID3v2Tag()) {
			ID3v2::FrameList l = mpegFile->ID3v2Tag()->frameListMap()["POPM"];
			if (!l.isEmpty()) {
				ID3v2::PopularimeterFrame *pf = static_cast<ID3v2::PopularimeterFrame*>(l.front());
				if (pf) {
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
			}
		} else if (mpegFile->ID3v1Tag()) {
			qDebug() << "FileHelper::rating: Not implemented for ID3v1Tag";
		}
		break;
	default:
		break;
	}
	return r;
}

/** Sets the inner picture. */
void FileHelper::setCover(Cover *cover)
{
	qDebug() << "FileHelper::setCover, cover==NULL?" << (cover == NULL);
	MPEG::File *mpegFile = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(_file);
		if (mpegFile->ID3v2Tag()) {
			// Look for picture frames only
			ID3v2::FrameList mp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			if (!mp3Frames.isEmpty()) {
				for (ID3v2::FrameList::Iterator it = mp3Frames.begin(); it != mp3Frames.end() ; it++) {
					// Removing a frame will invalidate any pointers on the list
					mpegFile->ID3v2Tag()->removeFrame(*it);
					qDebug() << "removing a frame";
					break;
				}
			}
			if (cover != NULL) {
				ByteVector bv(cover->byteArray().data(), cover->byteArray().length());
				qDebug() << "cover->hasChanged()" << cover->hasChanged();
				ID3v2::AttachedPictureFrame *pictureFrame = new ID3v2::AttachedPictureFrame();
				qDebug() << "cover.mimeType()" << QString(cover->mimeType());
				pictureFrame->setMimeType(cover->mimeType());
				pictureFrame->setPicture(bv);
				mpegFile->ID3v2Tag()->addFrame(pictureFrame);
				qDebug() << "adding a frame";
			}
		} else if (mpegFile->ID3v1Tag()) {
			qDebug() << "FileHelper::setCover: Not implemented for ID3v1Tag";
		}
		break;
	default:
		break;
	}
}

void FileHelper::setRating(int rating)
{
	MPEG::File *mpegFile = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(_file);
		if (mpegFile->ID3v2Tag()) {
			ID3v2::FrameList l = mpegFile->ID3v2Tag()->frameListMap()["POPM"];
			ID3v2::PopularimeterFrame *pf = NULL;
			if (l.isEmpty()) {
				pf = new ID3v2::PopularimeterFrame();
				mpegFile->ID3v2Tag()->addFrame(pf);
			} else {
				pf = static_cast<ID3v2::PopularimeterFrame*>(l.front());
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
		} else if (mpegFile->ID3v1Tag()) {
			qDebug() << "FileHelper::rating: Not implemented for ID3v1Tag";
		}
		break;
	default:
		break;
	}
	_file->save();
}

bool FileHelper::isValid() const
{
	return _file->isValid();
}

QString FileHelper::title() const
{
	return QString(_file->tag()->title().toCString(true));
}

QString FileHelper::trackNumber() const
{
	return QString("%1").arg(_file->tag()->track(), 2, 10, QChar('0')).toUpper();
}

QString FileHelper::album() const
{
	return QString(_file->tag()->album().toCString(true));
}

QString FileHelper::length() const
{
	return QString(QDateTime::fromTime_t(_file->audioProperties()->length()).toString("m:ss"));
}

QString FileHelper::artist() const
{
	return QString(_file->tag()->artist().toCString(true));
}

QString FileHelper::year() const
{
	return QString::number(_file->tag()->year());
}

QString FileHelper::genre() const
{
	return QString(_file->tag()->genre().toCString(true));
}

QString FileHelper::comment() const
{
	return QString(_file->tag()->comment().toCString(true));
}

bool FileHelper::save()
{
	return _file->save();
}

QString FileHelper::convertKeyToID3v2Key(QString key)
{
	/// TODO other relevant keys
	if (key.compare("ARTISTALBUM") == 0) {
		return "TPE2";
	} else {
		return "";
	}
}

QString FileHelper::extractFlacFeature(const QString &featureToExtract) const
{
	QString feature;
	FLAC::File *flacFile = static_cast<FLAC::File*>(_file);
	if (flacFile->ID3v2Tag()) {
		ID3v2::FrameList l = flacFile->ID3v2Tag()->frameListMap()[featureToExtract.toStdString().data()];
		if (!l.isEmpty()) {
			feature = QString(l.front()->toString().toCString(true));
		}
	} else if (flacFile->ID3v1Tag()) {
		qDebug() << "FileHelper::extractFlacFeature: Not yet implemented for ID3v1Tag FLAC file";
	} else if (flacFile->xiphComment()) {
		const Ogg::FieldListMap map = flacFile->xiphComment()->fieldListMap();
		if (!map[featureToExtract.toStdString().data()].isEmpty()) {
			feature = QString(map[featureToExtract.toStdString().data()].front().toCString(true));
		}
	}
	return feature;
}

QString FileHelper::extractMpegFeature(const QString &featureToExtract) const
{
	QString feature;
	MPEG::File *mpegFile = static_cast<MPEG::File*>(_file);
	if (mpegFile->ID3v2Tag()) {
		ID3v2::FrameList l = mpegFile->ID3v2Tag()->frameListMap()[featureToExtract.toStdString().data()];
		if (!l.isEmpty()) {
			feature = QString(l.front()->toString().toCString(true));
		}
	} else if (mpegFile->ID3v1Tag()) {
		qDebug() << "FileHelper::extractMpegFeature: Not yet implemented for ID3v1Tag MP3 file";
	}
	return feature;
}
