#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QDir>
#include <QFileInfo>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY MusicSearchEngine : public QObject
{
	Q_OBJECT

public:
	MusicSearchEngine(QObject *parent = 0);

public slots:
	void doSearch();

signals:
	/** A Jpeg or a PNG was found in observed directories. */
	///FIXME: however, it's not proven this file belongs to a well formed music directory like "<REP> -> Tracks + Cover.JPG"
	void scannedCover(const QString &);

	void scannedFile(const QString &);

	void progressChanged(const int &);

	void searchHasEnded();
};

#endif // MUSICSEARCHENGINE_H
