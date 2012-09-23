#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QFileInfo>
#include <QThread>

#include <QDir>

/*class MusicSearchEngine : public QThread*/
class MusicSearchEngine : public QObject
{
	Q_OBJECT

private:
	QList<QDir> savedLocations;

public:
	MusicSearchEngine(QObject *parent = 0);

public slots:
	void setLocations(const QList<QDir> &locations);

protected:
	//void run();

public slots:
	void doSearch();

signals:
	void scannedCover(const QString &);

	void scannedFile(int, const QString &);

	void progressChanged(const int &);

	void endSearch();
};

#endif // MUSICSEARCHENGINE_H
