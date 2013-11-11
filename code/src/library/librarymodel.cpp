#include "librarymodel.h"
#include "playlists/starrating.h"
#include "settings.h"

#include <QDir>
#include <QStandardPaths>

#include <QtDebug>

using namespace TagLib;

LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(0, 1, parent)
{
	_currentInsertPolicy = Settings::getInstance()->insertPolicy();
}

/** Removes everything. */
void LibraryModel::clear()
{
	_albums.clear();
	_artists.clear();
	_albums2.clear();
	_albumsAbsPath.clear();
	_artistsAlbums.clear();
	_discNumbers.clear();
	_years.clear();
	_letters.clear();
	_persistentItems.clear();
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

void LibraryModel::insertLetter(const QString &letters)
{	
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
void LibraryModel::addCoverPathToAlbum(const QString &fileName)
{
	QFileInfo fileInfo(fileName);
	LibraryItemAlbum *album = _albumsAbsPath.value(fileInfo.absolutePath());
	if (album && album->coverFileName().isEmpty()) {
		album->setCoverFileName(fileInfo.fileName());
		/// XXX: kind of hack
		//qDebug() << "addCoverPathToAlbum" << album->text() << fileInfo.fileName();
		album->persistentItem()->setCoverFileName(fileInfo.fileName());
		qDebug() << "new cover added for album:" << album->text() << fileName;
	}
}

/** Build a tree from a flat file saved on disk. */
void LibraryModel::loadFromFile()
{
	QFile mmmmp(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"));
	if (mmmmp.open(QIODevice::ReadOnly)) {

		QByteArray tracksByteArray = qUncompress(mmmmp.readAll());
		QDataStream inputDataStream(&tracksByteArray, QIODevice::ReadOnly);
		PersistentItem persistedItem;
		while (!inputDataStream.atEnd()) {
			persistedItem.read(inputDataStream);
			this->insertTrack(persistedItem.absoluteFilePath(), persistedItem.artist(), persistedItem.artistAlbum(), persistedItem.album(),
							  persistedItem.text(), persistedItem.trackNumber(), persistedItem.discNumber(), persistedItem.year());
			if (!persistedItem.coverFileName().isEmpty()) {
				qDebug() << "cover is not empty. adding from file " << persistedItem.absoluteFilePath();
				this->addCoverPathToAlbum(persistedItem.absolutePath() + '/' + persistedItem.coverFileName());
			} else {
				qDebug() << "cover is empty. skipping file" << persistedItem.absoluteFilePath();
			}
		}
		mmmmp.close();
		emit loadedFromFile();
	}
}

/** Read a file from the filesystem and adds it into the library. */
void LibraryModel::readFile(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	if (fh.file() != NULL && fh.file()->tag() != NULL && !fh.file()->tag()->isEmpty()) {
		Tag *tag = fh.file()->tag();
		QString artist = QString(tag->artist().toCString(true)).trimmed();
		QString artistAlbum = fh.artistAlbum();
		QString album = QString(tag->album().toCString(true)).trimmed();
		QString title = QString(tag->title().toCString(true)).trimmed();
		this->insertTrack(absFilePath, artist, artistAlbum, album, title, tag->track(), fh.discNumber(), tag->year());
	} else if (fh.file() == NULL) {
		qDebug() << "fh.file() == NULL" << absFilePath;
	} else if (fh.file()->tag() == NULL) {
		qDebug() << "fh.file()->tag() == NULL" << absFilePath;
	} else if (fh.file()->tag()->isEmpty()) {
		qDebug() << "fh.file()->tag()->isEmpty()" << absFilePath;
	}
}

void LibraryModel::insertTrack(const QString &absFilePath, const QString &artist, const QString &artistAlbum, const QString &album,
							   const QString &title, int trackNumber, int discNumber, int year)
{
	QString theArtist = artistAlbum.isEmpty() ? artist : artistAlbum;
	LibraryItemArtist *itemArtist = NULL;
	LibraryItemAlbum *itemAlbum = NULL;
	LibraryItemTrack *itemTrack = NULL;
	LibraryItem *itemYear = NULL;
	LibraryItemDiscNumber *itemDiscNumber = NULL;

	QFileInfo fileInfo(absFilePath);
	static bool existingArtist = true;
	switch (_currentInsertPolicy) {
	case Artist:
		// Level 1
		if (_artists.contains(theArtist.toLower())) {
			itemArtist = _artists.value(theArtist.toLower());
			existingArtist = true;
		} else {
			itemArtist = new LibraryItemArtist(theArtist);
			_artists.insert(theArtist.toLower(), itemArtist);
			invisibleRootItem()->appendRow(itemArtist);
			this->insertLetter(theArtist);
			existingArtist = false;
		}
		// Level 2
		if (existingArtist && _albums.contains(QPair<LibraryItemArtist*, QString>(itemArtist, album))) {
			itemAlbum = _albums.value(QPair<LibraryItemArtist*, QString>(itemArtist, album));
		} else {
			itemAlbum = new LibraryItemAlbum(album);
			qDebug() << "creating a new album by" << theArtist << ":" << album;
			_albums.insert(QPair<LibraryItemArtist *, QString>(itemArtist, album), itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3 (option)
		if (discNumber > 0 && !_discNumbers.contains(QPair<LibraryItemAlbum*, int>(itemAlbum, discNumber))) {
			itemDiscNumber = new LibraryItemDiscNumber(discNumber);
			_discNumbers.insert(QPair<LibraryItemAlbum *, int>(itemAlbum, discNumber), itemDiscNumber);
			itemAlbum->appendRow(itemDiscNumber);
		}
		// Level 3
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new LibraryItemTrack(title);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Album:
		// Level 1
		if (_albums2.contains(album)) {
			itemAlbum = _albums2.value(album);
		} else {
			itemAlbum = new LibraryItemAlbum(album);
			_albums2.insert(album, itemAlbum);
			invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(album);
		}
		// Level 2
		itemTrack = new LibraryItemTrack(title);
		itemAlbum->appendRow(itemTrack);
		break;
	case ArtistAlbum:
		// Level 1
		if (_albums2.contains(theArtist + album)) {
			itemAlbum = _albums2.value(theArtist + album);
		} else {
			itemAlbum = new LibraryItemAlbum(theArtist + " – " + album);
			_albums2.insert(theArtist + album, itemAlbum);
			invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(theArtist);
		}
		// Level 2
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new LibraryItemTrack(title);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Year:
		// Level 1
		if (_years.contains(year)) {
			itemYear = _years.value(year);
		} else {
			if (year > 0) {
				itemYear = new LibraryItem(QString::number(year));
			} else {
				itemYear = new LibraryItem();
			}
			_years.insert(year, itemYear);
			invisibleRootItem()->appendRow(itemYear);
		}
		// Level 2
		if (_albums2.contains(theArtist + album)) {
			itemAlbum = _albums2.value(theArtist + album);
		} else {
			itemAlbum = new LibraryItemAlbum(theArtist + " – " + album);
			_albums2.insert(theArtist + album, itemAlbum);
			itemYear->appendRow(itemAlbum);
		}
		// Level 3
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new LibraryItemTrack(title);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	}
	QString absolutePath = fileInfo.absolutePath();
	itemTrack->setAbsoluteFilePath(fileInfo.absolutePath(), fileInfo.fileName());
	itemTrack->setDiscNumber(discNumber);
	itemTrack->setTrackNumber(trackNumber);

	PersistentItem *persistentItem = new PersistentItem(itemTrack);
	if (itemAlbum != NULL && itemAlbum->persistentItem() == NULL && !_albumsAbsPath.contains(absolutePath)) {
		itemAlbum->setAbsolutePath(absolutePath);
		itemAlbum->setPersistentItem(persistentItem);
		itemAlbum->setYear(year);
		qDebug() << "creating a new album" << itemAlbum->text() << ", persist" << persistentItem->text();
		_albumsAbsPath.insert(absolutePath, itemAlbum);
	}
	if (itemAlbum) {
		persistentItem->setAlbum(album);
	}
	persistentItem->setArtist(artist);
	persistentItem->setArtistAlbum(theArtist);
	persistentItem->setYear(year);
	_persistentItems.insert(persistentItem);
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

		QByteArray tracksByteArray;
		QDataStream tracksDataStream(&tracksByteArray, QIODevice::ReadWrite);
		QSetIterator<PersistentItem*> it(_persistentItems);
		while (it.hasNext()) {
			it.next()->write(tracksDataStream);
		}
		mmmmp.write(qCompress(tracksByteArray, 9));
		mmmmp.close();
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
