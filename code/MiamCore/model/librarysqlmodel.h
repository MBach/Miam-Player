#ifndef LIBRARYSQLMODEL_H
#define LIBRARYSQLMODEL_H

#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QThread>
#include <QWeakPointer>

class FileHelper;
class MusicSearchEngine;

#include "miamcore_global.h"

class MIAMCORE_LIBRARY LibrarySqlModel : public QSqlTableModel
{
	Q_OBJECT
private:
	QThread _workerThread;

	MusicSearchEngine *_musicSearchEngine;

public:
	explicit LibrarySqlModel(const QSqlDatabase &db, QObject *parent = 0);

private:
	void loadFromFileDB();

public slots:
	void load();
	void rebuild();

private slots:
	/** Reads an external picture which is close to multimedia files (same folder). */
	void saveCoverRef(const QString &coverPath);

	/** Reads a file from the filesystem and adds it into the library. */
	void saveFileRef(const QString &absFilePath);

signals:
	void coverWasUpdated(const QFileInfo &);
	void progressChanged(const int &);
	void trackExtractedFromDB(const QSqlRecord &);
	void trackExtractedFromFS(const FileHelper &);
};

#endif // LIBRARYSQLMODEL_H
