#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QFileInfo>
#include <QMap>

#include <fileref.h>
#include <id3v2tag.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>

#include "filehelper.h"

#include "libraryitemalbum.h"
#include "libraryitemartist.h"
#include "libraryitemdiscnumber.h"
#include "libraryitemletter.h"
#include "libraryitemtrack.h"
#include "persistentitem.h"
#include "settings.h"

#include <QSet>
#include <QStandardItemModel>

class LibraryModel : public QStandardItemModel
{
	Q_OBJECT
private:
	QHash<QString, LibraryItemArtist*> _artists;
	QHash<QPair<LibraryItemArtist*, QString>, LibraryItemAlbum*> _albums;
	QHash<QPair<LibraryItemAlbum*, int>, LibraryItemDiscNumber*> _discNumbers;
	QHash<QString, LibraryItemAlbum*> _albums2;
	QHash<QString, LibraryItemAlbum*> _albumsAbsPath;
	QHash<QString, LibraryItemArtist*> _artistsAlbums;
	QHash<int, LibraryItem*> _years;
	QSet<QString> _letters;
	QSet<PersistentItem*> _persistentItems;
	Settings::InsertPolicy _currentInsertPolicy;

public:

	LibraryModel(QObject *parent = 0);

	/** Removes everything. */
	void clear();

	//void removeNode(const QModelIndex &index);

	LibraryItem *itemFromIndex(const QModelIndex &index) const {
		return static_cast<LibraryItem*>(QStandardItemModel::itemFromIndex(index));
	}

	inline Settings::InsertPolicy currentInsertPolicy() const { return _currentInsertPolicy; }

	inline void setInsertPolicy(Settings::InsertPolicy policy) { _currentInsertPolicy = policy; }

private:
	void insertLetter(const QString &letters);
	void insertTrack(const QString &absFilePath, const QString &artist, const QString &artistAlbum, const QString &album,
					 const QString &title, int trackNumber, int discNumber, int year);

signals:
	/** A flat file on your computer was successfully loaded. */
	void loadedFromFile();

public slots:
	/** Add (a path to) an icon to every album. */
	void addCoverPathToAlbum(const QString &fileName);

	/** Build a tree from a flat file saved on disk. */
	void loadFromFile();

	/** Read a file from the filesystem and adds it into the library. */
	void readFile(const QString &absFilePath);

	/** Save a tree to a flat file on disk. */
	void saveToFile();
};

#endif // LIBRARYMODEL_H
