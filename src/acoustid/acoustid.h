#ifndef ACOUSTID_H
#define ACOUSTID_H

#include "miamacoustid_global.hpp"
#include "qchromaprint.h"
#include "requestpool.h"
#include "matchingrecordswidget.h"

#include <QPushButton>

/**
 * \brief		The AcoustId class can fetch tags automatically from Webservice
 * \details		This class uses Chromaprint library and MusicBrainz
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMACOUSTID_LIBRARY AcoustId : public QObject
{
	Q_OBJECT
private:
	static QString _apiKey;
	static QString _wsAcoustID;

	RequestPool *_requestPool;
	QChromaprint *_chromaprint;
	MatchingRecordsWidget *_matchingRecordsWidget;
	QPushButton *_analyzeButton;

public:
	explicit AcoustId(QObject *parent = nullptr);

	virtual ~AcoustId();

	void start(const QList<QUrl> &tracks);

signals:
	void releaseFound(const MusicBrainz::Release &);
	void tracksAnalyzed();
};

#endif // ACOUSTID_H
