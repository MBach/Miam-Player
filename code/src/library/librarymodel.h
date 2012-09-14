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

class LibraryItem;

#include <QStandardItemModel>

class LibraryModel : public QStandardItemModel
{
	Q_OBJECT

private:
	QMap<QString, QStandardItem*> alphabeticalSeparators;
	QMap<QString, LibraryItem*> artists;
	QMap<QPair<LibraryItem*, QString>, LibraryItem*> albums;

	// An efficient way to tell if a track was already inserted
	QHash<QString, LibraryItem*> tracks;

	// A "cover" is not really a cover, it's just a reference to the upper folder where one track was scanned
	// For a track in ~/music/randomArtist/randomAlbum/track01.mp3, ~/music/randomArtist/randomAlbum is stored
	QMap<QString, LibraryItem*> covers;

	QMap<LibraryItem*, QIcon> albumsWithCovers;

	Q_ENUMS(MediaType)

public:
	LibraryModel(QObject *parent = 0);

	enum MediaType { LETTER	= Qt::UserRole+1,
					 ARTIST	= Qt::UserRole+2,
					 ALBUM	= Qt::UserRole+3,
					 TRACK	= Qt::UserRole+4
				   };

	/** Removes everything. */
	void clear();

	/** Artist? Album? */
	LibraryItem* hasArtist(const QString &artist) const;
	LibraryItem* hasAlbum(LibraryItem *artist, const QString &album) const;

	/** Insert a new artist/album/track in the library. */
	LibraryItem* insertArtist(const QString &artist);
	LibraryItem* insertAlbum(const QString &album, const QString &path, LibraryItem *parentArtist);
	void insertTrack(int musicLocationIndex, const QString &fileName, FileHelper &fileHelper, LibraryItem *parent);

	void makeSeparators();

	void removeNode(const QModelIndex &index);

private:
	/** Recursively reads the input stream to build nodes and append them to its parent. */
	void loadNode(QDataStream &in, LibraryItem *parent);

	/** Recursively writes nodes to the output stream. */
	void writeNode(QDataStream &dataStream, LibraryItem *parent);

signals:
	/** Tell the view that a new node was created, and needs to be associated with its delegate. */
	void associateNodeWithDelegate(LibraryItem*);

	/** A flat file on your computer was successfully loaded. */
	void loadedFromFile();

public slots:
	/** Add (a path to) an icon to every album. */
	void addCoverPathToAlbum(const QString &qFileName);

	/** If True, draws one cover before an album name. */
	void displayCovers(bool withCovers);

	/** Build a tree from a flat file saved on disk. */
	void loadFromFile();

	/** Read a file from the filesystem and adds it into the library. */
	void readFile(int musicLocationIndex, const QString &qFileName);

	/** Save a tree to a flat file on disk. */
	void saveToFile();
};

#endif // LIBRARYMODEL_H
