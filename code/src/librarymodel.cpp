#include "librarymodel.h"
#include "starrating.h"
#include "settings.h"
#include "libraryitem.h"

#include <QtDebug>


LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(parent)
{
	this->setColumnCount(1);
}

/** Removes everything. */
void LibraryModel::clear()
{
	albums.clear();
	albumsWithCovers.clear();
	alphabeticalSeparators.clear();
	artists.clear();
	covers.clear();
	tracks.clear();
	QStandardItemModel::clear();
}

/** Artist? */
LibraryItem* LibraryModel::hasArtist(const QString &artist) const
{
	return artists.value(artist);
}

/** Album? */
LibraryItem* LibraryModel::hasAlbum(LibraryItem* artist, const QString &album) const
{
	return albums.value(QPair<LibraryItem*, QString>(artist, album));
}

/** Insert a new artist in the library. */
LibraryItem* LibraryModel::insertArtist(const QString &artist)
{
	// Check if the first letter of the artist is a known new letter
	if (!artist.isEmpty()) {
		QString c = artist.left(1).toUpper();
		QString letter;
		if (c.contains(QRegExp("\\w"))) {
			letter = c;
		} else {
			/// How can I stick "Various" at the top of the tree view? (and NOT using this ugly trick)
			letter = tr(" Various");
		}
		if (!alphabeticalSeparators.contains(letter)) {
			LibraryItem *separator = new LibraryItem(letter);
			separator->setMediaType(LETTER);
			separator->setDelegate(new LibraryItemDelegate(this));
			alphabeticalSeparators.insert(letter, separator);
			this->appendRow(separator);
		}
	}

	// Create the artist
	LibraryItem *itemArtist = new LibraryItem(artist);
	itemArtist->setFilePath(artist);
	itemArtist->setMediaType(ARTIST);
	artists.insert(artist, itemArtist);
	this->appendRow(itemArtist);
	return itemArtist;
}

/** Insert a new album in the library. */
LibraryItem* LibraryModel::insertAlbum(const QString &album, const QString &path, LibraryItem *parentArtist)
{
	LibraryItem *itemAlbum = new LibraryItem(album);
	QString coverPath = path.left(path.lastIndexOf('/'));
	covers.insert(coverPath, itemAlbum);
	itemAlbum->setMediaType(ALBUM);
	parentArtist->setChild(parentArtist->rowCount(), itemAlbum);
	albums.insert(QPair<LibraryItem *, QString>(parentArtist, album), itemAlbum);
	return itemAlbum;
}

/** Insert a new track in the library. */
LibraryItem* LibraryModel::insertTrack(int musicLocationIndex, const QString &fileName, uint track, QString &title, LibraryItem *parent)
{
	QList<QVariant> musicLocations = Settings::getInstance()->musicLocations();
	// Check if a track was already inserted
	// Imagine if a one has added two music folders: ~/music/randomArtist and ~/music/randomArtist/randomAlbum
	// When iterating over directories and subdirectories, at some time, we will try to add twice the same album called "randomAlbum"
	// So tracks need to be compared each time, by saving their absolute file path in a non persistent map (tracks)
	bool isNewTrack = true;
	if (musicLocations.size() > 1) {
		for (int i=0; i < musicLocations.size(); i++) {
			QString musicLocation = musicLocations.at(i).toString();
			QString file = musicLocation.append(fileName);
			QStringList previousTrack = tracks.values(parent);
			if (previousTrack.contains(file)) {
				isNewTrack = false;
				break;
			} else {
				tracks.insertMulti(parent, file);
			}
		}
	}

	LibraryItem *itemTitle = NULL;
	if (isNewTrack) {
		itemTitle = new LibraryItem(title);
		itemTitle->setFilePath(musicLocationIndex, fileName);
		itemTitle->setMediaType(TRACK);
		itemTitle->setRating(track);
		itemTitle->setTrackNumber(track);
		itemTitle->setFont(Settings::getInstance()->font(Settings::LIBRARY));
		parent->setChild(parent->rowCount(), itemTitle);
	}
	return itemTitle;
}

/** Add (a path to) an icon to every album. */
void LibraryModel::addCoverPathToAlbum(const QString &qFileName)
{
	LibraryItem *indexAlbum = covers.value(qFileName.left(qFileName.lastIndexOf('/')));
	if (indexAlbum) {

		Settings *settings = Settings::getInstance();
		int i = indexAlbum->child(0, 0)->data(LibraryItem::IDX_TO_ABS_PATH).toInt();
		QVariant v = settings->musicLocations().at(i);
		indexAlbum->setFilePath(i, qFileName.mid(v.toString().size()+1));

		// Keep a copy of covers in case of any changes in settings
		albumsWithCovers.insert(indexAlbum, indexAlbum->icon());
	}
}

/** If True, draws one cover before an album name. */
void LibraryModel::displayCovers(bool withCovers)
{
	foreach (LibraryItem *album, albums.values()) {
		if (withCovers) {
			album->setIcon(albumsWithCovers.value(album));
		} else {
			album->setIcon(QIcon());
		}
	}
	Settings::getInstance()->setCovers(withCovers);
}

/** Save a tree to a flat file on disk. */
void LibraryModel::saveToFile()
{
	QFile mmmmp("library.mmmmp");
	if (mmmmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QByteArray output;
		QDataStream dataStream(&output, QIODevice::ReadWrite);

		// Write the root first, which is not a LibraryItem instance
		QStandardItem *item = invisibleRootItem();
		dataStream << item->rowCount();
		for (int i=0; i<item->rowCount(); i++) {
			this->writeNode(dataStream, dynamic_cast<LibraryItem *>(item->child(i, 0)));
		}
		mmmmp.write(qCompress(output, 9));
		mmmmp.close();
	}
}

/** Build a tree from a flat file saved on disk. */
void LibraryModel::loadFromFile()
{
	QFile mmmmp("library.mmmmp");
	if (mmmmp.open(QIODevice::ReadOnly)) {
		QByteArray input = qUncompress(mmmmp.readAll());
		QDataStream dataStream(&input, QIODevice::ReadOnly);

		// To build the first item, just read how many children the root has
		quint32 rootChildren;
		dataStream >> rootChildren;
		bool separators = Settings::getInstance()->toggleSeparators();
		for (quint32 i=0; i < rootChildren; i++) {
			LibraryItem *libraryItem = new LibraryItem();
			libraryItem->read(dataStream);
			if (separators || libraryItem->mediaType() != LETTER) {
				appendRow(libraryItem);
			}

			// Then build nodes
			this->loadNode(dataStream, libraryItem);
		}
		mmmmp.close();
		emit loadedFromFile();
	}
}

/** Recursively reads the input stream to build nodes and append them to its parent. */
void LibraryModel::loadNode(QDataStream &in, LibraryItem *parent)
{
	int mediaType = parent->mediaType();
	if (mediaType == LibraryModel::ARTIST || mediaType == LibraryModel::ALBUM) {
		int childCount = parent->data(LibraryItem::CHILD_COUNT).toInt();
		for (int i=0; i < childCount; i++) {
			LibraryItem *node = new LibraryItem();
			node->read(in);
			parent->appendRow(node);

			// Tell the view that a new node was created, and needs to be associated with its delegate
			emit associateNodeWithDelegate(node);

			// Then load nodes
			this->loadNode(in, node);
		}
	}
}

/** Recursively writes nodes to the output stream. */
void LibraryModel::writeNode(QDataStream &dataStream, LibraryItem *parent)
{
	if (parent->hasChildren()) {
		// If the current item has children. It needs to be recursively parsed until there's a leaf
		for (int i=0; i < parent->rowCount(); i++) {
			// Store once the number of children and the new artist/album
			if (i == 0) {
				parent->write(dataStream);
			}
			this->writeNode(dataStream, parent->child(i, 0));
		}
	} else {
		// A track needs to be saved
		parent->write(dataStream);
	}
}
