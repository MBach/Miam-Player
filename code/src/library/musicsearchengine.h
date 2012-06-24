#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QFileInfo>
#include <QThread>

class MusicSearchEngine : public QThread
{
	Q_OBJECT

public:
	MusicSearchEngine(QObject *parent = 0);

protected:
	void run();

signals:
	void scannedCover(const QString &);

	void scannedFile(int, const QString &);

	void progressChanged(const int &);

public slots:

};

#endif // MUSICSEARCHENGINE_H
