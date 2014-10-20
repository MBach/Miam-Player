#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY MusicSearchEngine : public QObject
{
	Q_OBJECT
private:
	QFileSystemWatcher *_watcher;

public:
	MusicSearchEngine(QObject *parent = 0);

public slots:
	void doSearch();

signals:
	/** A JPG or a PNG was found next to a valid audio file in the same directory. */
	void scannedCover(const QString &, const QString &);

	void scannedFile(const QString &);

	void progressChanged(const int &);

	void searchHasEnded();
};

#endif // MUSICSEARCHENGINE_H
