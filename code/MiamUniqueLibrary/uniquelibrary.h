#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QWidget>

#include "miamuniquelibrary_global.h"
#include "model/sqldatabase.h"

namespace Ui {
class UniqueLibrary;
}

class FlowLayout;

class AlbumForm;

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget
{
	Q_OBJECT

private:
	Ui::UniqueLibrary *ui;
	FlowLayout *_flowLayout;
	SqlDatabase *_db;

	QMap<QString, AlbumForm*> _albums;

public:
	explicit UniqueLibrary(QWidget *parent = 0);

	void init(SqlDatabase *db);

	void insertTrackFromRecord(const QSqlRecord &record);
	void insertTrackFromFile(const FileHelper &fh);

private:
	void insertTrack(const QString &absFilePath, const QString &, const QString &, const QString &, int, const QString &, int year);

private slots:
	void reset();
	void updateCover(const QFileInfo &);
};

#endif // UNIQUELIBRARY_H
