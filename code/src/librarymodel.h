#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QFileInfo>
#include <QMap>

class LibraryItem;

#include <QStandardItemModel>

class LibraryModel : public QStandardItemModel
{
	Q_OBJECT

private:
	QMap<QString, LibraryItem*> alphabeticalSeparators;
	QMap<QString, LibraryItem*> artists;
	QMap<QString, LibraryItem*> albums;

	// A "cover" is not really a cover, it's just a reference to the upper folder where one track was scanned
	// For a track in ~/music/randomArtist/randomAlbum/track01.mp3, ~/music/randomArtist/randomAlbum is stored
	QMap<QString, LibraryItem*> covers;

	QMap<LibraryItem*, QIcon> albumsWithCovers;

	Q_ENUMS(MediaType)

public:
	LibraryModel(QObject *parent = 0);

	enum MediaType { LETTER = 0, ARTIST = 1, ALBUM = 2, TRACK = 3, LENGTH = 4 };

	/** Removes everything. */
	void clear();

	/** Artist? Album? */
	LibraryItem* hasArtist(const QString &artist) const;
	LibraryItem* hasAlbum(const QString &album) const;
	LibraryItem* hasCover(const QString &cover) const;

	/** Insert a new artist/album/track in the library. */
	LibraryItem* insertArtist(const QString &artist);
	LibraryItem* insertAlbum(const QString &album, const QString &path, LibraryItem *parentArtist);
	LibraryItem* insertTrack(int musicLocationIndex, const QString &fileName, uint track, QString &title, LibraryItem *parent);

	/** Add an icon to every album, if exists. */
	void insertAlbumIcon(QIcon &icon, LibraryItem *album);

	/// TEST
	//inline QModelIndexList persistentIndexList () const { return QStandardItemModel::persistentIndexList(); }
	//inline void changePersistentIndexList(const QModelIndexList & from, const QModelIndexList & to ) { QStandardItemModel::changePersistentIndexList(from, to); }

private:
	/** Recursively reads the input stream to build nodes and append them to its parent. */
	void loadNode(QDataStream &in, LibraryItem *parent);

	/** Recursively writes nodes to the output stream. */
	void writeNode(QDataStream &dataStream, LibraryItem *parent);

signals:
	/** Tell the view that a new node was created, and needs to be associated with its delegate. */
	void associateNodeWithDelegate(int);

	void loadedFromFile();

public slots:
	/** If True, draws one cover before an album name. */
	void setIcon(bool withCovers);

	/** Save a tree to a flat file on disk. */
	void saveToFile();

	/** Build a tree from a flat file saved on disk. */
	void loadFromFile();
};

#endif // LIBRARYMODEL_H
