#ifndef LIBRARYSQLMODEL_H
#define LIBRARYSQLMODEL_H

#include <QtSql>

#include "musicsearchengine.h"

#include "filehelper.h"

class MIAMCORE_LIBRARY LibrarySqlModel : public QSqlTableModel
{
	Q_OBJECT
private:
	QSqlDatabase _db;

	MusicSearchEngine *_musicSearchEngine;

public:
	explicit LibrarySqlModel(QObject *parent = 0);

	void loadFromFileDB();

public slots:
	void rebuild();

private slots:
	/** Read a file from the filesystem and adds it into the library. */
	void readFile(const QString &absFilePath);

	void saveDB();

signals:
	void trackCreated(const FileHelper &);
};

#endif // LIBRARYSQLMODEL_H
