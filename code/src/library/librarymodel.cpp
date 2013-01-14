#include "librarymodel.h"
#include "starrating.h"
#include "settings.h"
#include "libraryitem.h"

#include <QtDebug>

using namespace TagLib;

LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(parent)
{
	this->setColumnCount(1);
	//this->setSortRole(LibraryItem::INTERNAL_NAME);
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
	return artists.value(artist.toLower());
}

/** Album? */
LibraryItem* LibraryModel::hasAlbum(LibraryItem* artist, const QString &album) const
{
	return albums.value(QPair<LibraryItem*, QString>(artist, album));
}

/** Insert a new artist in the library. */
LibraryItem* LibraryModel::insertArtist(const QString &artist)
{
	// Create the artist
	LibraryItem *itemArtist = new LibraryItem(artist, ARTIST);
	itemArtist->setFilePath(artist);
	artists.insert(artist.toLower(), itemArtist);
	this->appendRow(itemArtist);
	return itemArtist;
}

/** Insert a new album in the library. */
LibraryItem* LibraryModel::insertAlbum(const QString &album, const QString &path, LibraryItem *parentArtist)
{
	LibraryItem *itemAlbum = new LibraryItem(album, ALBUM);
	QString coverPath = path.left(path.lastIndexOf('/'));
	covers.insert(coverPath, itemAlbum);
	parentArtist->setChild(parentArtist->rowCount(), itemAlbum);
	albums.insert(QPair<LibraryItem *, QString>(parentArtist, album), itemAlbum);
	return itemAlbum;
}

/** Insert a new track in the library. */
void LibraryModel::insertTrack(int musicLocationIndex, const QString &fileName, FileHelper &fileHelper, LibraryItem *parent)
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
				tracks.insert(file, parent);
			}
		}
	}

	LibraryItem *itemTitle = NULL;
	if (isNewTrack) {
		QString title(fileHelper.file()->tag()->title().toCString(false));
		if (title.isEmpty()) {
			title = QFileInfo(fileName).baseName();
		}
		itemTitle = new LibraryItem(title, TRACK, fileHelper.type());
		itemTitle->setFilePath(musicLocationIndex, fileName);
		itemTitle->setRating(fileHelper.file()->tag()->track());
		itemTitle->setTrackNumber(fileHelper.file()->tag()->track());
		itemTitle->setFont(Settings::getInstance()->font(Settings::LIBRARY));
		parent->setChild(parent->rowCount(), itemTitle);
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
				LibraryItem *separator = new LibraryItem(letter, LETTER);
				alphabeticalSeparators.insert(letter, separator);
			}
		}
	}
	root->appendRows(alphabeticalSeparators.values());
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
		QStandardItem *root = invisibleRootItem();

		for (quint32 i=0; i < rootChildren; i++) {
			LibraryItem *libraryItem = new LibraryItem();
			libraryItem->read(dataStream);
			if (separators || libraryItem->type() != LETTER) {
				root->appendRow(libraryItem);
			}

			// Then build nodes
			this->loadNode(dataStream, libraryItem);
		}
		mmmmp.close();
		/// FIXME
		qDebug() << "loadFromFile" << artists.count();
		emit loadedFromFile();
	}
}

/** Read a file from the filesystem and adds it into the library. */
void LibraryModel::readFile(int musicLocationIndex, const QString &qFileName)
{
	static LibraryItem *indexArtist = NULL;
	static LibraryItem *indexAlbum = NULL;
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
		indexArtist = hasArtist(QString(artist.toCString(false)));
		if (indexArtist == NULL) {
			indexArtist = insertArtist(QString(artist.toCString(false)));
		}

		// Is there is already an album from this artist?
		indexAlbum = hasAlbum(indexArtist, QString(f->tag()->album().toCString(false)));
		if (indexAlbum == NULL) {
			// New album to create, only if it's not empty
			if (f->tag()->album().isEmpty()) {
				indexAlbum = indexArtist;
			} else {
				indexAlbum = insertAlbum(QString(f->tag()->album().toCString(false)), filePath, indexArtist);
			}
		}
		this->insertTrack(musicLocationIndex, qFileName, fh, indexAlbum);
	}
	delete f;
}

/** Save a tree to a flat file on disk. */
void LibraryModel::saveToFile()
{
	QFile mmmmp("library.mmmmp");
	if (mmmmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QByteArray output;

		// No need to store separators, they will be rebuilt at runtime.
		QStandardItem *item = invisibleRootItem();
		int separators = 0;
		if (Settings::getInstance()->toggleSeparators()) {
			for (int i = 0; i < item->rowCount(); i++) {
				if (item->child(i)->data(LibraryItem::MEDIA_TYPE).toInt() == LibraryModel::LETTER) {
					++separators;
				}
			}
		}

		// Write the root first, which is not a LibraryItem instance
		QDataStream dataStream(&output, QIODevice::ReadWrite);
		dataStream << item->rowCount() - separators;
		for (int i = 0; i < item->rowCount() - separators; i++) {
			this->writeNode(dataStream, static_cast<LibraryItem *>(item->child(i, 0)));
		}
		mmmmp.write(qCompress(output, 9));
		mmmmp.close();
	}
}

/** Recursively remove a leaf and its parents if the leaf is a "one node" branch. */
void LibraryModel::removeNode(const QModelIndex &index)
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
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
	QVariant d;
	switch (role) {
	case LibraryItem::INTERNAL_NAME:
		if (index.isValid()) {
			qDebug() << "INTERNAL_NAME" << index.data(LibraryItem::INTERNAL_NAME);

		}
		d = QStandardItemModel::data(index, role);
		break;
	default:
		//qDebug() << "default";
		d = QStandardItemModel::data(index, role);
		break;
	}
	return d;
}

/** Recursively reads the input stream to build nodes and append them to its parent. */
void LibraryModel::loadNode(QDataStream &in, LibraryItem *parent)
{
	int type = parent->type();
	if (type == LibraryModel::ARTIST || type == LibraryModel::ALBUM) {
		int childCount = parent->data(LibraryItem::CHILD_COUNT).toInt();
		for (int i = 0; i < childCount; i++) {
			LibraryItem *node = new LibraryItem();
			node->read(in);
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
		if (parent->type() == LibraryModel::TRACK) {
			parent->write(dataStream);
		}
	}
}
