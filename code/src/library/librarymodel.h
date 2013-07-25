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



//class LibraryItem;

#include "libraryitemalbum.h"
#include "libraryitemartist.h"
#include "libraryitemletter.h"
#include "libraryitemtrack.h"

#include <QSet>
#include <QStandardItemModel>

class LibraryModel : public QStandardItemModel
{
	Q_OBJECT

private:
	QMap<QString, QStandardItem*> _alphabeticalSeparators;
	QMap<QString, LibraryItemArtist*> _artists;
	QMap<QPair<LibraryItemArtist*, QString>, LibraryItemAlbum*> _albums;

	// An efficient way to tell if a track was already inserted
	QHash<QString, LibraryItemAlbum*> _tracks;

	// A "cover" is not really a cover, it's just a reference to the upper folder where one track was scanned
	// For a track in ~/music/randomArtist/randomAlbum/track01.mp3, ~/music/randomArtist/randomAlbum is stored
	QMap<QString, LibraryItemAlbum*> _covers;

	QMap<LibraryItemAlbum*, QIcon> _albumsWithCovers;

	enum InsertPolicy { Artist,
						  Album,
						  ArtistAlbum,
						  Year,
						  Folders };

	InsertPolicy _currentInsertPolicy;

	/// test
	QSet<TagLib::FileRef*> _fileRefs;

public:
	LibraryModel(QObject *parent = 0);

	/** Removes everything. */
	void clear();

	/** Artist? Album? */
	LibraryItemArtist *hasArtist(const QString &artist) const;
	LibraryItemAlbum* hasAlbum(LibraryItemArtist *artist, const QString &album) const;

	/** Insert a new artist/album/track in the library. */
	LibraryItemArtist* insertArtist(const QString &artist);
	LibraryItemAlbum* insertAlbum(const QString &album, const QString &path, LibraryItemArtist *parentArtist);
	void insertTrack(int musicLocationIndex, const QString &fileName, FileHelper &fileHelper, LibraryItemAlbum *parent);

	void makeSeparators();

	//void removeNode(const QModelIndex &index);

	 LibraryItem *itemFromIndex(const QModelIndex &index) const {
		 return static_cast<LibraryItem*>(QStandardItemModel::itemFromIndex(index));
	 }

	 QMap<QString, LibraryItemArtist*> artists() const { return _artists; }

private:
	/** Recursively reads the input stream to build nodes and append them to its parent. */
	void loadNode(QDataStream &in, LibraryItem *parent);

	/** Recursively writes nodes to the output stream. */
	void writeNode(QDataStream &dataStream, LibraryItem *parent);

	void insertTrack2(TagLib::Tag* tag, InsertPolicy policy);

signals:
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

	/// Work in progress
	void readFile2(const QString &qFileName);

	/** Save a tree to a flat file on disk. */
	void saveToFile();
};

#endif // LIBRARYMODEL_H
