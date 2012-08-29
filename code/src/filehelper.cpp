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

#include <id3v2tag.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>

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
		fileType = 0;
	} else if (suffix == "asf") {
		f = new ASF::File(fp);
		fileType = 1;
	} else if (suffix == "flac") {
		f = new FLAC::File(fp);
		fileType = 2;
	} else if (suffix == "m4a") {
		f = new MP4::File(fp);
		fileType = 4;
	} else if (suffix == "mpc") {
		f = new MPC::File(fp);
		fileType = 5;
	} else if (suffix == "mp3") {
		f = new MPEG::File(fp);
		fileType = 6;
	} else if (suffix == "ogg" || suffix == "oga") {
		f = new Vorbis::File(fp);
		fileType = 7;
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
	case 0:
		apeFile = dynamic_cast<APE::File*>(f);
		//artAlb = "not yet for ape";
		break;
	case 1:
		asfFile = dynamic_cast<ASF::File*>(f);
		//artAlb = "not yet for asf";
		break;
	case 2:
		flacFile = dynamic_cast<FLAC::File*>(f);
		//artAlb = "not yet for flac";
		break;
	case 3:
		break;
	case 4:
		mp4File = dynamic_cast<MP4::File*>(f);
		//artAlb = "not yet for mp4";
		break;
	case 5:
		mpcFile = dynamic_cast<MPC::File*>(f);
		//artAlb = "not yet for mpc";
		break;
	case 6:
		// For albums with multiple Artists, like OST, the "TPE2" value is commonly used for the tag "Album Artist"
		// It is used in Windows 7, foobar2000, etc
		mpegFile = dynamic_cast<MPEG::File*>(f);
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
	case 7:
		oggFile = dynamic_cast<Ogg::File*>(f);
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

bool FileHelper::insert(QString key, QString value)
{
	// Standard tags
	String v = value.toStdString();
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
	} else {
		// Other non generic tags, like Artist Album
		APE::File *apeFile = NULL;
		ASF::File *asfFile = NULL;
		FLAC::File *flacFile = NULL;
		MP4::File *mp4File = NULL;
		MPC::File *mpcFile = NULL;
		MPEG::File *mpegFile = NULL;

		switch (fileType) {
		case 0:
			apeFile = dynamic_cast<APE::File*>(f);
			qDebug() << "APE file";
			break;
		case 1:
			asfFile = dynamic_cast<ASF::File*>(f);
			qDebug() << "ASF file";
			break;
		case 2:
			flacFile = dynamic_cast<FLAC::File*>(f);
			qDebug() << "FLAC file";
			break;
		case 3:
			qDebug() << "Mod file";
			break;
		case 4:
			mp4File = dynamic_cast<MP4::File*>(f);
			qDebug() << "MP4 file";
			break;
		case 5:
			mpcFile = dynamic_cast<MPC::File*>(f);
			qDebug() << "MPC file";
			break;
		case 6:
			mpegFile = dynamic_cast<MPEG::File*>(f);
			if (mpegFile->ID3v2Tag()) {
				ID3v2::Tag *tag = mpegFile->ID3v2Tag();
				if (tag) {
					QString convertedKey = this->convertKeyToID3v2Key(key);
					ID3v2::FrameList l = tag->frameListMap()[convertedKey.toStdString().data()];
					if (!l.isEmpty()) {
						tag->removeFrame(l.front());
					}
					ID3v2::TextIdentificationFrame *tif = new ID3v2::TextIdentificationFrame(ByteVector(convertedKey.toStdString().data()));
					tif->setText(value.toStdString().data());
					tag->addFrame(tif);
				}
			} else if (mpegFile->ID3v1Tag()) {
				qDebug() << "ID3v1Tag";
			}
			break;
		case 7:
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
