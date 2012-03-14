#include <QDebug>

#include "treeitem.h"
#include "librarymodel.h"
#include "starrating.h"

#include "settings.h"

#include "libraryitem.h"

LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(parent)
{
	//LibraryItem *header = new LibraryItem("Artists");
	this->setColumnCount(1);
	//this->setHeaderData(0, Qt::Horizontal, QVariant("Artists"), Qt::DisplayRole);
	//this->setHorizontalHeaderItem(0, header);
	//setHorizontalHeaderLabels(QStringList("Artists"));
}

/** Removes everything. */
void LibraryModel::clear()
{
	albums.clear();
	albumsWithCovers.clear();
	alphabeticalSeparators.clear();
	artists.clear();
	covers.clear();
	QStandardItemModel::clear();
}

/** Artist? */
LibraryItem* LibraryModel::hasArtist(const QString &artist) const
{
	return artists.value(artist);
}

/** Album? */
LibraryItem* LibraryModel::hasAlbum(const QString &album) const
{
	return albums.value(album);
}

/** Cover? */
LibraryItem* LibraryModel::hasCover(const QString &cover) const
{
	return covers.value(cover);
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
	covers.insert(path.left(path.lastIndexOf('/')), itemAlbum);
	itemAlbum->setMediaType(ALBUM);
	parentArtist->setChild(parentArtist->rowCount(), itemAlbum);
	albums.insert(album, itemAlbum);
	return itemAlbum;
}

/** Insert a new track in the library. */
LibraryItem* LibraryModel::insertTrack(int musicLocationIndex, const QString &fileName, uint track, QString &title, LibraryItem *parent)
{
	//QString title = QString("%1. ").arg(fileRef.tag()->track(), (int)2, (int)10, (const QChar)'0');
	LibraryItem *itemTitle = new LibraryItem(title);
	itemTitle->setFilePath(musicLocationIndex, fileName);
	itemTitle->setMediaType(TRACK);
	itemTitle->setRating(track);
	itemTitle->setTrackNumber(track);

	Settings *settings = Settings::getInstance();
	itemTitle->setFont(settings->font(Settings::LIBRARY));

	parent->setChild(parent->rowCount(), itemTitle);
	return itemTitle;
}

/** Add an icon to every album, if exists. */
void LibraryModel::insertAlbumIcon(QIcon &icon, LibraryItem *album)
{
	album->setIcon(icon);
	// Keep a copy of covers in case of any changes in settings
	albumsWithCovers.insert(album, icon);
}

/** If True, draws one cover before an album name. */
void LibraryModel::setIcon(bool withCovers)
{
	foreach (LibraryItem *album, albums.values()) {
		if (withCovers) {
			album->setIcon(albumsWithCovers.value(album));
		} else {
			album->setIcon(QIcon());
		}
	}
	// Notify the view that the model has changed, so it needs to be updated
	//emit itemChanged(invisibleRootItem());
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
		int childCount = parent->data(Qt::UserRole+3).toInt();
		for (int i=0; i < childCount; i++) {
			LibraryItem *node = new LibraryItem();
			node->read(in);
			parent->appendRow(node);

			// Tell the view that a new node was created, and needs to be associated with its delegate
			emit associateNodeWithDelegate(node->row());

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
