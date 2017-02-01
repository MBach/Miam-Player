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

QChromaprint::QChromaprint(AcoustIdPlugin *parent)
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
	const char *file_name = file.toStdString().data();
	double ts = 0.0;

	// Real-time audio stream
	/*if (g_abs_ts) {
		ts = GetCurrentTimestamp();
	}*/

	/*if (!strcmp(file_name, "-")) {
		file_name = "pipe:0";
	}*/

	/*if (!reader.Open(file_name)) {
		qDebug() << "Cannot open file: " << file;
		return 0;
	}*/

	AVDemuxer demuxer;
	demuxer.setMedia(file);
	if (!demuxer.load()) {
		qWarning() << "Failed to load file " << demuxer.fileName();
		return 0;
	}

	QScopedPointer<AudioDecoder> dec(AudioDecoder::create()); // delete by user
	dec->setCodecContext(demuxer.audioCodecContext());
	//dec->prepare();
	if (!dec->open())
		qFatal("open decoder error");

	int astream = demuxer.audioStream();
	Packet pkt;
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
		//if (ao->isOpen()) {
		//	af = ao->audioFormat();
		//} else {
			//ao->setAudioFormat(af);
			//dec->resampler()->setOutAudioFormat(ao->audioFormat());
			// if decoded format is not supported by audio renderer, change decoder output format
			//if (af != ao->audioFormat())
			//	dec->resampler()->prepare();
			// now af is supported by audio renderer. it's safe to open
			//if (!ao->open())
			//	qFatal("Open audio output error");
#if 0 // always resample ONCE due to QtAV bug
			// the first format unsupported frame still need to be converted to a supported format
			if (!ao->isSupported(frame.format()))
				frame = frame.to(af);
#endif
			qDebug() << "Input: " << frame.format();
			qDebug() << "Output: " << af;
		//}
		qDebug() << "playing: " << frame.timestamp();
		fflush(0);
		// always resample ONCE. otherwise data are all 0x0. QtAV bug
		frame = frame.to(af);
		/*QByteArray data(frame.data()); // plane data. currently only packet sample formats are supported.
		while (!data.isEmpty()) {
			ao->play(QByteArray::fromRawData(data.constData(), qMin(data.size(), ao->bufferSize())));
			data.remove(0, qMin(data.size(), ao->bufferSize()));
		}*/
	}


	/*if (!chromaprint_start(_ctx, _acoustId->localPlayer()->audio()->sampleRate(), _acoustId->localPlayer()->audio()->channels())) {
		qDebug() << "Could not initialize the fingerprinting process";
		return 0;
	}

	size_t stream_size = 0;
	const size_t stream_limit = g_max_duration * _acoustId->localPlayer()->audio()->sampleRate();

	size_t chunk_size = 0;
	const size_t chunk_limit = g_max_chunk_duration * _acoustId->localPlayer()->audio()->sampleRate();

	size_t extra_chunk_limit = 0;
	double overlap = 0.0;
	if (chunk_limit > 0 && g_overlap) {
		extra_chunk_limit = chromaprint_get_delay(_ctx);
		overlap = chromaprint_get_delay_ms(_ctx) / 1000.0;
	}

	bool first_chunk = true;
	bool read_failed = false;
	bool got_results = false;

	//_acoustId->localPlayer()->audio()

	while (!reader.IsFinished()) {
		const int16_t *frame_data = nullptr;
		size_t frame_size = 0;
		if (!reader.Read(&frame_data, &frame_size)) {
			fprintf(stderr, "ERROR: %s\n", reader.GetError().c_str());
			read_failed = true;
			break;
		}

		bool stream_done = false;
		if (stream_limit > 0) {
			const auto remaining = stream_limit - stream_size;
			if (frame_size > remaining) {
				frame_size = remaining;
				stream_done = true;
			}
		}
		stream_size += frame_size;

		if (frame_size == 0) {
			if (stream_done) {
				break;
			} else {
				continue;
			}
		}

		bool chunk_done = false;
		size_t first_part_size = frame_size;
		if (chunk_limit > 0) {
			const auto remaining = chunk_limit + extra_chunk_limit - chunk_size;
			if (first_part_size > remaining) {
				first_part_size = remaining;
				chunk_done = true;
			}
		}

		if (!chromaprint_feed(_ctx, frame_data, first_part_size * _acoustId->localPlayer()->audio()->sampleRate())) {
			fprintf(stderr, "ERROR: Could not process audio data\n");
			exit(2);
		}

		chunk_size += first_part_size;

		if (chunk_done) {
			if (!chromaprint_finish(_ctx)) {
				fprintf(stderr, "ERROR: Could not finish the fingerprinting process\n");
				exit(2);
			}

			const auto chunk_duration = (chunk_size - extra_chunk_limit) * 1.0 / _acoustId->localPlayer()->audio()->sampleRate() + overlap;
			//PrintResult(_ctx, reader, first_chunk, ts, chunk_duration);
			got_results = true;

			//if (g_abs_ts) {
			//	ts = GetCurrentTimestamp();
			//} else {
			//	ts += chunk_duration;
			//}

			if (g_overlap) {
				if (!chromaprint_clear_fingerprint(_ctx)) {
					fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
					return 0;
				}
				ts -= overlap;
			} else {
				if (!chromaprint_start(_ctx, _acoustId->localPlayer()->audio()->sampleRate(), _acoustId->localPlayer()->audio()->channels())) {
					fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
					return 0;
				}
			}

			if (first_chunk) {
				extra_chunk_limit = 0;
				first_chunk = false;
			}

			chunk_size = 0;
		}

		frame_data += first_part_size * _acoustId->localPlayer()->audio()->sampleRate();
		frame_size -= first_part_size;

		if (frame_size > 0) {
			if (!chromaprint_feed(_ctx, frame_data, frame_size * _acoustId->localPlayer()->audio()->sampleRate())) {
				fprintf(stderr, "ERROR: Could not process audio data\n");
				return 0;
			}
		}

		chunk_size += frame_size;

		if (stream_done) {
			break;
		}
	}

	if (!chromaprint_finish(_ctx)) {
		fprintf(stderr, "ERROR: Could not finish the fingerprinting process\n");
		return 0;
	}

	if (chunk_size > 0) {
		const auto chunk_duration = (chunk_size - extra_chunk_limit) * 1.0 / _acoustId->localPlayer()->audio()->sampleRate() + overlap;
		//PrintResult(_ctx, reader, first_chunk, ts, chunk_duration);
		got_results = true;
	} else if (first_chunk) {
		fprintf(stderr, "ERROR: Not enough audio data\n");
		return 0;
	}

//	if (!g_ignore_errors) {
//		if (read_failed) {
//			exit(got_results ? 3 : 2);
//		}
//	}
	*/
	return 1;
}
