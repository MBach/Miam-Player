#include "librarymodel.h"
#include "playlists/starrating.h"
#include "settings.h"

#include "library/libraryitemfactory.h"

#include <QDir>
#include <QStandardPaths>

#include <QtDebug>

using namespace TagLib;

LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(0, 1, parent)
{}

/** Removes everything. */
void LibraryModel::clear()
{
	albums.clear();
	albumsWithCovers.clear();
	alphabeticalSeparators.clear();
	artists.clear();
	covers.clear();
	tracks.clear();
	removeRows(0, rowCount());
}

/** Artist? */
LibraryItemArtist* LibraryModel::hasArtist(const QString &artist) const
{
	return artists.value(artist.toLower());
}

/** Album? */
LibraryItemAlbum *LibraryModel::hasAlbum(LibraryItemArtist* artist, const QString &album) const
{
	return albums.value(QPair<LibraryItemArtist*, QString>(artist, album));
}

/** Insert a new artist in the library. */
LibraryItemArtist *LibraryModel::insertArtist(const QString &artist)
{
	// Create the artist
	LibraryItemArtist *itemArtist = new LibraryItemArtist(artist);
	itemArtist->setFilePath(artist);
	artists.insert(artist.toLower(), itemArtist);
	this->appendRow(itemArtist);
	return itemArtist;
}

/** Insert a new album in the library. */
LibraryItemAlbum *LibraryModel::insertAlbum(const QString &album, const QString &path, LibraryItemArtist *parentArtist)
{
	LibraryItemAlbum *itemAlbum = new LibraryItemAlbum(album);
	QString coverPath = path.left(path.lastIndexOf('/'));
	covers.insert(coverPath, itemAlbum);
	parentArtist->setChild(parentArtist->rowCount(), itemAlbum);
	albums.insert(QPair<LibraryItemArtist *, QString>(parentArtist, album), itemAlbum);
	return itemAlbum;
}

/** Insert a new track in the library. */
void LibraryModel::insertTrack(int musicLocationIndex, const QString &fileName, FileHelper &fileHelper, LibraryItemAlbum *parentAlbum)
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
			if (tracks.contains(file)) {
				isNewTrack = false;
				break;
			} else {
				tracks.insert(file, parentAlbum);
			}
		}
	}

	LibraryItemTrack *itemTitle = NULL;
	if (isNewTrack) {
		QString title(fileHelper.file()->tag()->title().toCString(true));
		if (title.isEmpty()) {
			title = QFileInfo(fileName).baseName();
		}
		itemTitle = new LibraryItemTrack(title, fileHelper.type());
		itemTitle->setFilePath(musicLocationIndex, fileName);
		//itemTitle->setRating(fileHelper.file()->tag()->track());
		itemTitle->setTrackNumber(fileHelper.file()->tag()->track());
		itemTitle->setFont(Settings::getInstance()->font(Settings::LIBRARY));

		parentAlbum->setChild(parentAlbum->rowCount(), itemTitle);
		if (parentAlbum->year() < 0) {
			parentAlbum->setYear(fileHelper.file()->tag()->year());
		}
	}
}

void LibraryModel::makeSeparators()
{
	// Removing previous separators
	QMapIterator<QString, QStandardItem*> mapIterator(alphabeticalSeparators);
	while (mapIterator.hasNext()) {
		mapIterator.next();
		LibraryItem *libraryItem = static_cast<LibraryItem*>(mapIterator.value());
		if (libraryItem) {
			this->removeRow(libraryItem->row(), libraryItem->index().parent());
		}
	}
	alphabeticalSeparators.clear();

	// Builing new separators
	QStandardItem *root = invisibleRootItem();
	for (int i = 0; i < root->rowCount(); i++) {
		QString artist = root->child(i)->data(Qt::DisplayRole).toString();

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
				LibraryItemLetter *separator = new LibraryItemLetter(letter);
				alphabeticalSeparators.insert(letter, separator);
			}
		}
	}
	root->appendRows(alphabeticalSeparators.values());
}

/** Add (a path to) an icon to every album. */
void LibraryModel::addCoverPathToAlbum(const QString &qFileName)
{
	LibraryItemAlbum *indexAlbum = covers.value(qFileName.left(qFileName.lastIndexOf('/')));
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
	foreach (LibraryItemAlbum *album, albums.values()) {
		if (withCovers) {
			album->setIcon(albumsWithCovers.value(album));
		} else {
			album->setIcon(QIcon());
		}
	}
	Settings::getInstance()->setCovers(withCovers);
}

/** Build a tree from a flat file saved on disk. */
void LibraryModel::loadFromFile()
{
	QFile mmmmp(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"));
	if (mmmmp.open(QIODevice::ReadOnly)) {
		QByteArray input = qUncompress(mmmmp.readAll());
		QDataStream dataStream(&input, QIODevice::ReadOnly);

		// To build the first item, just read how many children the root has
		quint32 rootChildren;
		dataStream >> rootChildren;
		bool separators = Settings::getInstance()->toggleSeparators();
		QStandardItem *root = invisibleRootItem();

		for (quint32 i = 0; i < rootChildren; i++) {
			int type;
			dataStream >> type;
			qDebug() << type;
			LibraryItem *libraryItem = LibraryItemFactory::createItem(type);
			if (libraryItem) {
				libraryItem->read(dataStream);
				if (separators || libraryItem->type() != LibraryItem::Letter) {
					root->appendRow(libraryItem);
				}

				// Then build nodes
				this->loadNode(dataStream, libraryItem);
			}
		}
		mmmmp.close();
		/// FIXME ?
		emit loadedFromFile();
	}
}

/** Read a file from the filesystem and adds it into the library. */
void LibraryModel::readFile(int musicLocationIndex, const QString &qFileName)
{
	static LibraryItemArtist *indexArtist = NULL;
	static LibraryItemAlbum *indexAlbum = NULL;
	Settings *settings = Settings::getInstance();
	settings->musicLocations().at(musicLocationIndex).toString();
	QString filePath = settings->musicLocations().at(musicLocationIndex).toString() + qFileName;

	FileHelper fh(filePath);
	File *f = fh.file();

	if (f->isValid() && f->tag()) {

		String artist;
		String artistAlbum = fh.artistAlbum();
		if (artistAlbum.isEmpty()) {
			artist = f->tag()->artist();
		} else {
			artist = artistAlbum;
		}

		// Is there is already this artist in the library?
		indexArtist = hasArtist(QString(artist.toCString(true)));
		if (indexArtist == NULL) {
			indexArtist = insertArtist(QString(artist.toCString(true)));
		}

		// Is there is already an album from this artist?
		indexAlbum = hasAlbum(indexArtist, QString(f->tag()->album().toCString(true)));
		if (indexAlbum == NULL) {
			// New album to create, only if it's not empty
			if (f->tag()->album().isEmpty()) {
				indexAlbum = new LibraryItemAlbum("");
			} else {
				indexAlbum = insertAlbum(QString(f->tag()->album().toCString(true)), filePath, indexArtist);
			}
		}
		this->insertTrack(musicLocationIndex, qFileName, fh, indexAlbum);
	}
	delete f;
}

/** Save a tree to a flat file on disk. */
void LibraryModel::saveToFile()
{
	QString librarySaveFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	bool isFolderAvailable = true;
	QDir folder;
	if (!folder.exists(librarySaveFolder)) {
		isFolderAvailable = folder.mkpath(librarySaveFolder);
	}
	QFile mmmmp(librarySaveFolder.append(QDir::separator()).append("library.mmmmp"));
	if (isFolderAvailable && mmmmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

		// No need to store separators, they will be rebuilt at runtime.
		QStandardItem *item = invisibleRootItem();
		int separators = 0;
		if (Settings::getInstance()->toggleSeparators()) {
			for (int i = 0; i < item->rowCount(); i++) {
				if (item->child(i)->type() == LibraryItem::Letter) {
					++separators;
				}
			}
		}

		// Write the root first, which is not a LibraryItem instance
		QByteArray output;
		QDataStream dataStream(&output, QIODevice::ReadWrite);
		dataStream << item->rowCount() - separators;
		for (int i = 0; i < item->rowCount() - separators; i++) {
			this->writeNode(dataStream, static_cast<LibraryItem *>(item->child(i, 0)));
		}
		mmmmp.write(qCompress(output, 9));
		mmmmp.close();
	} else {
		qDebug() << "error when retrieving folder for saving useful data";
	}
}

/** Recursively remove a leaf and its parents if the leaf is a "one node" branch. */
/*void LibraryModel::removeNode(const QModelIndex &index)
{
	QModelIndex parent;
	if (this->rowCount(index.parent()) == 1) {
		parent = index.parent();
	}
	QStandardItem *qStandardItem = itemFromIndex(index);
	if (qStandardItem) {
		LibraryItem *libraryItem = static_cast<LibraryItem*>(qStandardItem);
		QPair<LibraryItem*, QString> pair;
		//QString albumPath;
		if (libraryItem) {
			QString key;
			switch (libraryItem->type()) {
			case TRACK:
				key = tracks.key(libraryItem);
				tracks.remove(key);
				break;
			case ALBUM:
				/// FIXME
				//albumPath = covers.key(libraryItem);
				//qDebug() << albumPath << covers.remove(albumPath);
				pair = albums.key(libraryItem);
				albums.remove(pair);
				albumsWithCovers.remove(libraryItem);
				break;
			case ARTIST:
				QString artist = artists.key(libraryItem);
				artists.remove(artist);
				break;
			}
			delete libraryItem;
		}
	}
	this->removeRow(index.row(), index.parent());
	if (parent.isValid()) {
		this->removeNode(parent);
	}
}*/

/** Recursively reads the input stream to build nodes and append them to its parent. */
void LibraryModel::loadNode(QDataStream &in, LibraryItem *parent)
{
	int type = parent->type();
	if (type == LibraryItem::Artist || type == LibraryItem::Album) {
		int childCount = parent->data(LibraryItem::CHILD_COUNT).toInt();
		for (int i = 0; i < childCount; i++) {
			in >> type;
			LibraryItem *node = LibraryItemFactory::createItem(type);
			node->read(in);
			//qDebug() << type << node->data(Qt::DisplayRole).toString();
			parent->appendRow(node);

			// Then load nodes recursively
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
		if (parent->type() == LibraryItem::Track) {
			parent->write(dataStream);
		}
	}
}
