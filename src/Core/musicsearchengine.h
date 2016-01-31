#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include "miamcore_global.h"

/**
 * \brief		The MusicSearchEngine class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MusicSearchEngine : public QObject
{
	Q_OBJECT
private:
	QTimer *_timer;
	QStringList _delta;

public:
	MusicSearchEngine(QObject *parent = nullptr);

	static bool isScanning;

	void setWatchForChanges(bool b);

public slots:
	void doSearch();

private slots:
	void watchForChanges();

signals:
	/** A JPG or a PNG was found next to a valid audio file in the same directory. */
	void scannedCover(const QString &, const QString &);

	void scannedFile(const QString &);

	void progressChanged(int);

	void searchHasEnded();
};

#endif // MUSICSEARCHENGINE_H
