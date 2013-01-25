#include "filehelper.h"

#include <QFileInfo>

#include <apefile.h>
#include <asffile.h>
#include <flacfile.h>
#include <mpcfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <vorbisfile.h>

#include <id3v2tag.h>
#include <id3v2frame.h>
#include <attachedPictureFrame.h>

#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>

#include <QImage>
#include <QtDebug>

using namespace TagLib;

QStringList FileHelper::suff = QStringList() << "ape" << "asf" << "flac" << "m4a" << "mpc" << "mp3" << "oga" << "ogg";

FileHelper::FileHelper(FileRef &fileRef, QVariant v)
	: fileType(v.toInt())
{
	f = fileRef.file();
}

FileHelper::FileHelper(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	QString suffix = fileInfo.suffix().toLower();
	const char *fp = filePath.toLocal8Bit().data();

	if (suffix == "ape") {
		f = new APE::File(fp);
		fileType = APE;
	} else if (suffix == "asf") {
		f = new ASF::File(fp);
		fileType = ASF;
	} else if (suffix == "flac") {
		f = new FLAC::File(fp);
		fileType = FLAC;
	} else if (suffix == "m4a") {
		f = new MP4::File(fp);
		fileType = MP4;
	} else if (suffix == "mpc") {
		f = new MPC::File(fp);
		fileType = MPC;
	} else if (suffix == "mp3") {
		f = new MPEG::File(fp);
		fileType = MP3;
	} else if (suffix == "ogg" || suffix == "oga") {
		f = new Vorbis::File(fp);
		fileType = OGG;
	} else {
		f = NULL;
		fileType = -1;
	}
}

String FileHelper::artistAlbum() const
{
	String artAlb = "";
	APE::File *apeFile = NULL;
	ASF::File *asfFile = NULL;
	FLAC::File *flacFile = NULL;
	MPC::File *mpcFile = NULL;
	MP4::File *mp4File = NULL;
	MPEG::File *mpegFile = NULL;
	Ogg::File *oggFile = NULL;

	switch (fileType) {
	case APE:
		apeFile = static_cast<APE::File*>(f);
		//artAlb = "not yet for ape";
		break;
	case ASF:
		asfFile = static_cast<ASF::File*>(f);
		//artAlb = "not yet for asf";
		break;
	case FLAC:
		flacFile = static_cast<FLAC::File*>(f);
		//artAlb = "not yet for flac";
		break;
	case 3:
		break;
	case MP4:
		mp4File = static_cast<MP4::File*>(f);
		//artAlb = "not yet for mp4";
		break;
	case MPC:
		mpcFile = static_cast<MPC::File*>(f);
		//artAlb = "not yet for mpc";
		break;
	case MP3:
		// For albums with multiple Artists, like OST, the "TPE2" value is commonly used for the tag "Album Artist"
		// It is used in Windows 7, foobar2000, etc
		mpegFile = static_cast<MPEG::File*>(f);
		if (mpegFile->ID3v2Tag()) {
			ID3v2::Tag *tag = mpegFile->ID3v2Tag();
			if (tag) {
				ID3v2::FrameList l = tag->frameListMap()["TPE2"];
				if (!l.isEmpty()) {
					artAlb = l.front()->toString();
				}
			}
		} else if (mpegFile->ID3v1Tag()) {

		}
		break;
	case OGG:
		oggFile = static_cast<Ogg::File*>(f);
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	}
	return artAlb;
}

bool FileHelper::insert(QString key, const QVariant &value)
{
	// Standard tags
	String v = value.toString().toStdString();

	/// XXX Create an enumeration somewhere
	if (key == "ALBUM") {
		f->tag()->setAlbum(v);
	} else if (key == "ARTIST") {
		f->tag()->setArtist(v);
	} else if (key == "COMMENT") {
		f->tag()->setComment(v);
	} else if (key == "GENRE") {
		f->tag()->setGenre(v);
	} else if (key == "TITLE") {
		f->tag()->setTitle(v);
	} else if (key == "TRACKNUMBER") {
		f->tag()->setTrack(value.toInt());
	} else if (key == "YEAR") {
		f->tag()->setYear(value.toInt());
	} else if (key == "COVER") {
		this->replaceCover(value);
	} else {
		// Other non generic tags, like Artist Album
		APE::File *apeFile = NULL;
		ASF::File *asfFile = NULL;
		FLAC::File *flacFile = NULL;
		MP4::File *mp4File = NULL;
		MPC::File *mpcFile = NULL;
		MPEG::File *mpegFile = NULL;

		switch (fileType) {
		case APE:
			apeFile = static_cast<APE::File*>(f);
			qDebug() << "APE file";
			break;
		case ASF:
			asfFile = static_cast<ASF::File*>(f);
			qDebug() << "ASF file";
			break;
		case FLAC:
			flacFile = static_cast<FLAC::File*>(f);
			qDebug() << "FLAC file";
			break;
		case 3:
			qDebug() << "Mod file";
			break;
		case MP4:
			mp4File = static_cast<MP4::File*>(f);
			qDebug() << "MP4 file";
			break;
		case MPC:
			mpcFile = static_cast<MPC::File*>(f);
			qDebug() << "MPC file";
			break;
		case MP3:
			mpegFile = static_cast<MPEG::File*>(f);
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
		case 8:
			qDebug() << "RIFF file";
			break;
		case 9:
			qDebug() << "TrueAudio file";
			break;
		case 10:
			qDebug() << "WavPack file";
			break;
		}
	}
	return true;
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

bool FileHelper::save()
{
	return f->save();
}

QByteArray FileHelper::extractCover()
{
	MPEG::File *mpegFile = NULL;
	QByteArray byteArray;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(f);
		if (mpegFile->ID3v2Tag()) {
			// Look for picture frames only
			ID3v2::FrameList listOfMp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			// It's possible to have more than one picture per file!
			if (!listOfMp3Frames.isEmpty()) {
				for (ID3v2::FrameList::ConstIterator it = listOfMp3Frames.begin(); it != listOfMp3Frames.end() ; it++) {
					// Cast a Frame* to AttachedPictureFrame*
					ID3v2::AttachedPictureFrame *pictureFrame = static_cast<ID3v2::AttachedPictureFrame*>(*it);
					// Performs a deep copy of the cover
					byteArray = QByteArray(pictureFrame->picture().data(), pictureFrame->picture().size());
				}
			}
		} else if (mpegFile->ID3v1Tag()) {
			qDebug() << "FileHelper::extractCover: Not implemented for ID3v1Tag";
		}
		break;
	default:
		break;
	}
	return byteArray;
}

void FileHelper::replaceCover(const QVariant &value)
{
	MPEG::File *mpegFile = NULL;
	switch (fileType) {
	case MP3:
		mpegFile = static_cast<MPEG::File*>(f);
		if (mpegFile->ID3v2Tag()) {
			// Look for picture frames only
			ID3v2::FrameList mp3Frames = mpegFile->ID3v2Tag()->frameListMap()["APIC"];
			if (!mp3Frames.isEmpty()) {
				for (ID3v2::FrameList::Iterator it = mp3Frames.begin(); it != mp3Frames.end() ; it++) {
					// Removing a frame will invalidate any pointers on the list
					mpegFile->ID3v2Tag()->removeFrame(*it);
					break;
				}
			}
			QByteArray b = value.toByteArray();
			if (!b.isEmpty()) {
				unsigned int l = (unsigned int)b.length();
				ByteVector bv(b.data(), l);
				qDebug() << "b.isEmpty()" << b.isEmpty() << b.size();
				qDebug() << "bv.isEmpty()" << bv.isEmpty() << bv.size();
				ID3v2::AttachedPictureFrame *pictureFrame = new ID3v2::AttachedPictureFrame();
				pictureFrame->setMimeType("image/jpeg");
				pictureFrame->setPicture(bv);
				mpegFile->ID3v2Tag()->addFrame(pictureFrame);
			}
		} else if (mpegFile->ID3v1Tag()) {
			qDebug() << "FileHelper::replaceCover: Not implemented for ID3v1Tag";
		}
		break;
	default:
		break;
	}
}
