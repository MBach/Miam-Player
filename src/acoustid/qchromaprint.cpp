#include "qchromaprint.h"

#include <QDir>
#include <stdio.h>

#include "acoustid.h"

//extern "C" {
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libavutil/opt.h>

//#if defined(HAVE_SWRESAMPLE)
#include <libswresample/swresample.h>
//#elif defined(HAVE_AVRESAMPLE)
//#include <libavresample/avresample.h>
//#endif
//}

//static Format g_format = TEXT;
//static char *g_input_format = nullptr;
int QChromaprint::g_input_channels = 0;
int QChromaprint::g_input_sample_rate = 0;
double QChromaprint::g_max_duration = 120;
double QChromaprint::g_max_chunk_duration = 0;
bool QChromaprint::g_overlap = false;
bool QChromaprint::g_raw = false;
bool QChromaprint::g_abs_ts = false;

#include <QtAV/AVPlayer.h>
#include <QtAV/AVDecoder.h>
#include <QtAV/AVDemuxer.h>
#include <QtAV/AudioDecoder.h>
#include <QScopedPointer>

using namespace QtAV;

#include <QtDebug>

QChromaprint::QChromaprint(AcoustId *parent)
	: QObject(parent)
	, _acoustId(parent)
	, _maxLength(120)
	, _duration(0)
{
	_ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
	//av_register_all();
}

QChromaprint::~QChromaprint()
{
	if (_ctx) {
		chromaprint_dealloc(_ctx);
	}
}

int QChromaprint::start(const QString &file)
{
	int r = this->processFile(file);
	qDebug() << Q_FUNC_INFO << r << _duration;
	return r;
}

QString QChromaprint::fingerprint() const
{
	char *fp = nullptr;
	int r = chromaprint_get_fingerprint(_ctx, &fp);
	if (r == 1) {
		QString s(fp);
		qDebug() << Q_FUNC_INFO << "decoded fingerprint" << s;
		return s;
	} else {
		return QString();
	}
}

QString QChromaprint::rawFingerprint() const
{
	int fsize;
	uint32_t *raw_fingerprint;
	/*int r =*/ chromaprint_get_raw_fingerprint(_ctx, &raw_fingerprint, &fsize);
	for (int i = 0; i < fsize; i++) {
		qDebug() << Q_FUNC_INFO << raw_fingerprint[i];
	}
	return QString();
}

uint QChromaprint::maxLength() const
{
	uint l = static_cast<uint>(_duration);
	if (l < _maxLength) {
		return l;
	} else {
		return _maxLength;
	}
}

/** This function has been extracted and modified from fpcalc.c example. */
int QChromaprint::processFile(const QString &file)
{
	AVDemuxer demuxer;
	demuxer.setMedia(file);
	if (!demuxer.load()) {
		qWarning() << "Failed to load file " << demuxer.fileName();
		return 0;
	}

	QScopedPointer<AudioDecoder> dec(AudioDecoder::create()); // delete by user
	dec->setCodecContext(demuxer.audioCodecContext());
	//dec->prepare();
	if (!dec->open()) {
		qWarning() << Q_FUNC_INFO << "open decoder error";
	}
	_duration = demuxer.duration() / 1000;

	int astream = demuxer.audioStream();
	Packet pkt;
	//int remaining = maxLength() * codec_ctx->channels * codec_ctx->sample_rate;
	unsigned long long remaining = -1;
	bool lastChunk = false;
	while (!demuxer.atEnd()) {
		if (!pkt.isValid()) { // continue to decode previous undecoded data
			if (!demuxer.readFrame() || demuxer.stream() != astream)
				continue;
			pkt = demuxer.packet();
		}
		if (!dec->decode(pkt)) {
			pkt = Packet(); // set invalid to read from demuxer
			continue;
		}
		// decode the rest data in the next loop. read from demuxer if no data remains
		pkt.data = QByteArray::fromRawData(pkt.data.constData() + pkt.data.size() - dec->undecodedSize(), dec->undecodedSize());
		AudioFrame frame(dec->frame()); // why is faster to call frame() for hwdec? no frame() is very slow for VDA
		if (!frame)
			continue;
		//frame.setAudioResampler(dec->resampler()); // if not set, always create a resampler in AudioFrame.to()
		AudioFormat af(frame.format());

		qDebug() << "playing: " << frame.timestamp();
		// always resample ONCE. otherwise data are all 0x0. QtAV bug
		frame = frame.to(af);

		/// Behold!
		const int16_t *frame_data = (const int16_t *)frame.data().data();

		int length = frame.channelCount() * frame.samplesPerChannel();

		if (remaining == -1) {
			remaining = maxLength() * frame.channelCount() * frame.samplesPerChannel();
		}
		if (maxLength() > 0) {
			if (remaining < length) {
				length = remaining;
				lastChunk = true;
			}
		}
		remaining -= length;
		qDebug() << "length: " << length;

		if (!chromaprint_feed(_ctx, frame_data, length)) {
			qDebug() << "Could not process audio data";
		}
		if (lastChunk) {
			break;
		}
	}
	if (!chromaprint_finish(_ctx)) {
		qDebug() << "Fingerprint calculation failed";
		return 0;
	}
	return 1;
}
