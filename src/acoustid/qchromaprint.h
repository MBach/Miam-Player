#ifndef QCHROMAPRINT_H
#define QCHROMAPRINT_H

#include <chromaprint/chromaprint.h>

#include <QObject>

#include "miamacoustid_global.hpp"

class AcoustId;

/**
 * \brief		The QChromaprint class wraps the Chromaprint Library.
 * \details		This class can generate fingerprints from files
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMACOUSTID_LIBRARY QChromaprint : public QObject
{
	Q_OBJECT
private:
	AcoustId *_acoustId;
	uint _maxLength;
	ChromaprintContext *_ctx;
	int _duration;

	static int g_input_channels;
	static int g_input_sample_rate;
	static double g_max_duration;
	static double g_max_chunk_duration;
	static bool g_overlap;
	static bool g_raw;
	static bool g_abs_ts;

public:
	explicit QChromaprint(AcoustId *parent);

	virtual ~QChromaprint();

	inline int duration() const { return _duration; }

	int start(const QString &file);

	QString fingerprint() const;
	QString rawFingerprint() const;

private:
	uint maxLength() const;

	/** This function has been extracted and modified from fpcalc.c example. */
	int processFile(const QString &file);

signals:
	void finished(const QString &fingerprint);
};

#endif // QCHROMAPRINT_H
