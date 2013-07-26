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
{
	_currentInsertPolicy = Album;
}

/** Removes everything. */
void LibraryModel::clear()
{
	_albums.clear();
	_albumsWithCovers.clear();
	_artists.clear();
	_covers.clear();
	_tracks.clear();
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

void LibraryModel::insertLetter(const QString &letters)
{	
	static QSet<QString> _letters;

	if (!letters.isEmpty()) {
		QString c = letters.left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		QString letter;
		if (c.contains(QRegExp("\\w"))) {
			letter = c;
		} else {
			/// How can I stick "Various" at the top of the tree view? (and NOT using this ugly trick)
			letter = tr(" Various");
		}
		if (!_letters.contains(letter)) {
			_letters.insert(letter);
			invisibleRootItem()->appendRow(new LibraryItemLetter(letter));
		}
	}
}

/** Add (a path to) an icon to every album. */
void LibraryModel::addCoverPathToAlbum(const QString &qFileName)
{
	LibraryItemAlbum *indexAlbum = _covers.value(qFileName.left(qFileName.lastIndexOf('/')));
	if (indexAlbum) {
		indexAlbum->setCoverPath(qFileName);

		// Keep a copy of covers in case of any changes in settings
		_albumsWithCovers.insert(indexAlbum, indexAlbum->icon());
	} else {
		qDebug() << "vide";
	}
}

/** If True, draws one cover before an album name. */
void LibraryModel::displayCovers(bool withCovers)
{
	foreach (LibraryItemAlbum *album, _albums.values()) {
		if (withCovers) {
			album->setIcon(_albumsWithCovers.value(album));
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
		QStandardItem *root = invisibleRootItem();

		for (quint32 i = 0; i < rootChildren; i++) {
			int type;
			dataStream >> type;
			LibraryItem *libraryItem = LibraryItemFactory::createItem(type);
			if (libraryItem) {
				libraryItem->read(dataStream);
				if (libraryItem->type() != LibraryItem::Letter) {
					root->appendRow(libraryItem);
				}

				// Then build nodes
				this->loadNode(dataStream, libraryItem);
			}
		}
		mmmmp.close();
		emit loadedFromFile();
	}
}

/** Read a file from the filesystem and adds it into the library. */
void LibraryModel::readFile(const QString &qFileName)
{
	FileRef *fileRef = new FileRef(QFile::encodeName(qFileName).constData());
	if (fileRef && fileRef->tag()) {
		_fileRefs.insert(fileRef);
		insertTrack(fileRef->tag(), _currentInsertPolicy);
	}
}

/// Strategy or Policy? Strategies are usually called from other classes
void LibraryModel::insertTrack(Tag *tag, InsertPolicy policy)
{
	LibraryItemArtist *itemArtist = NULL;
	LibraryItemAlbum *itemAlbum = NULL;
	LibraryItemTrack *itemTrack = NULL;
	LibraryItem *itemYear = NULL;
	QString artist(tag->artist().toCString(true));
	QString album(tag->album().toCString(true));
	QString title(tag->title().toCString(true));

	static QMap<QString, LibraryItemAlbum*> _albums2;
	static QMap<QString, LibraryItemArtist*> _artistsAlbums;
	static QMap<int, LibraryItem*> _years;

	switch (policy) {
	case Artist:
		// Level 1
		if (_artists.contains(artist.toLower())) {
			itemArtist = _artists.value(artist.toLower());
		} else {
			itemArtist = new LibraryItemArtist(artist);
			_artists.insert(artist.toLower(), itemArtist);
			invisibleRootItem()->appendRow(itemArtist);
			this->insertLetter(artist);
		}
		// Level 2
		itemAlbum = _albums.value(QPair<LibraryItemArtist*, QString>(itemArtist, album));
		if (itemAlbum == NULL) {
			itemAlbum = new LibraryItemAlbum(album);
			itemAlbum->setYear(tag->year());
			_albums.insert(QPair<LibraryItemArtist *, QString>(itemArtist, album), itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3
		itemTrack = new LibraryItemTrack(title, -1);
		itemAlbum->appendRow(itemTrack);
		break;
	case Album:
		// Level 1
		if (_albums2.contains(album)) {
			itemAlbum = _albums2.value(album);
		} else {
			itemAlbum = new LibraryItemAlbum(album);
			itemAlbum->setYear(tag->year());
			_albums2.insert(album, itemAlbum);
			invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(album);
		}
		// Level 2
		itemTrack = new LibraryItemTrack(title, -1);
		itemAlbum->appendRow(itemTrack);
		break;
	case ArtistAlbum:
		// Level 1
		if (_artistsAlbums.contains(artist + album)) {
			itemArtist = _artistsAlbums.value(artist + album);
		} else {
			itemArtist = new LibraryItemArtist(artist + " - " + album);
			_artistsAlbums.insert(artist + album, itemArtist);
			invisibleRootItem()->appendRow(itemArtist);
			this->insertLetter(artist);
		}
		// Level 2
		itemTrack = new LibraryItemTrack(title, -1);
		itemArtist->appendRow(itemTrack);
		break;
	case Year:
		// Level 1
		if (_years.contains(tag->year())) {
			itemYear = _years.value(tag->year());
		} else {
			if (tag->year() > 0) {
				itemYear = new LibraryItem(QString::number(tag->year()));
			} else {
				itemYear = new LibraryItem();
			}
			_years.insert(tag->year(), itemYear);
			invisibleRootItem()->appendRow(itemYear);
		}
		// Level 2
		if (_artistsAlbums.contains(artist + album)) {
			itemArtist = _artistsAlbums.value(artist + album);
		} else {
			itemArtist = new LibraryItemArtist(artist + " - " + album);
			_artistsAlbums.insert(artist + album, itemArtist);
			itemYear->appendRow(itemArtist);
		}
		// Level 3
		itemTrack = new LibraryItemTrack(title, -1);
		itemArtist->appendRow(itemTrack);
		break;
	case Folders:
		break;
	}
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
		for (int i = 0; i < item->rowCount(); i++) {
			if (item->child(i)->type() == LibraryItem::Letter) {
				++separators;
			}
		}

		// Write the root first, which is not a LibraryItem instance
		QByteArray output;
		QDataStream dataStream(&output, QIODevice::ReadWrite);
		dataStream << item->rowCount() - separators;
		for (int i = 0; i < item->rowCount(); i++) {
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
		for (int i = 0; i < parent->childCount(); i++) {
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
