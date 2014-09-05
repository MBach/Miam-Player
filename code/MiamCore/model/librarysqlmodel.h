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

/**
 * \brief		The LibrarySqlModel class is used to query (CRUD) the table 'Tracks' in database.
 * \details		This class gather all tracks informations and put them in database. It's also connected to views by sending tracks informations.
 *				This class owns a worker that is able to search on hard drive for Audio Files.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY LibrarySqlModel : public QSqlTableModel
{
	Q_OBJECT
private:
	/** This worker is used to avoid a blocking UI when scanning the FileSystem. */
	QThread _workerThread;

	/** Object than can iterate throught the FileSystem for Audio files. */
	MusicSearchEngine *_musicSearchEngine;

public:
	/** Default constructor. */
	explicit LibrarySqlModel(const QSqlDatabase &db, QObject *parent = 0);

	/**
	 * Update a list of tracks. If track name has changed, will be removed from Library then added right after.
	 * \param tracksToUpdate 'First' in pair is actual filename, 'Second' is the new filename, but may be empty.*/
	void updateTracks(const QList<QPair<QString, QString>> &tracksToUpdate);

private:
	/** Read all tracks entries in the database and send them to connected views. */
	void loadFromFileDB();

public slots:
	/** Load an existing database file or recreate it, if not found. */
	void load();

	/** Safe delete and recreate from scratch (table Tracks only). */
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
