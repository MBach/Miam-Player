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
	//_albumsWithCovers.clear();
	_artists.clear();
	//_covers.clear();
	_albums2.clear();
	_artistsAlbums.clear();
	_years.clear();
	_letters.clear();
	_tracks.clear();
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
/// FIXME
/*void LibraryModel::addCoverPathToAlbum(const QString &fileName)
{
	// LibraryItemAlbum *indexAlbum = _covers.value(fileName);
	// "D:\Musique\MP3\Air\2001 - 10000 Hz Legend\Folder.jpg"
	LibraryItemAlbum *indexAlbum = _tracks.value(QFileInfo(fileName).absolutePath());
	if (indexAlbum) {
		indexAlbum->setCoverPath(fileName);

		// Keep a copy of covers in case of any changes in settings
		_albumsWithCovers.insert(indexAlbum, indexAlbum->icon());
	} else {
		qDebug() << "vide";
	}
}*/

/** If True, draws one cover before an album name. */
/*
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
*/

/** Build a tree from a flat file saved on disk. */
void LibraryModel::loadFromFile()
{
	QFile mmmmp(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"));
	if (mmmmp.open(QIODevice::ReadOnly)) {
		QByteArray input = qUncompress(mmmmp.readAll());
		QDataStream dataStream(&input, QIODevice::ReadOnly);
		LibraryItemTrack tempTrack;
		while (!dataStream.atEnd()) {
			tempTrack.read(dataStream);
			this->insertTrack(tempTrack.filePath(), tempTrack.artist(), tempTrack.artistAlbum(), tempTrack.album(),
							  tempTrack.text(), tempTrack.trackNumber(), tempTrack.year());
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
		this->insertTrackFromFileSystem(absFilePath, fh);
	} else if (fh.file() == NULL) {
		qDebug() << "fh.file() == NULL" << absFilePath;
	} else if (fh.file()->tag() == NULL) {
		qDebug() << "fh.file()->tag() == NULL" << absFilePath;
	} else if (fh.file()->tag()->isEmpty()) {
		qDebug() << "fh.file()->tag()->isEmpty()" << absFilePath;
	}
}

void LibraryModel::insertTrack(const QString &absFilePath, const QString &artist, const QString &artistAlbum, const QString &album,
							   const QString &title, int trackNumber, int year)
{
	QString theArtist = artistAlbum.isEmpty() ? artist : artistAlbum;
	LibraryItemArtist *itemArtist = NULL;
	LibraryItemAlbum *itemAlbum = NULL;
	LibraryItemTrack *itemTrack = NULL;
	LibraryItem *itemYear = NULL;
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
			itemAlbum->setYear(year);
			_albums.insert(QPair<LibraryItemArtist *, QString>(itemArtist, album), itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3
		if (artistAlbum.isEmpty()) {
			itemTrack = new LibraryItemTrack(title, -1);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")", -1);
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Album:
		// Level 1
		if (_albums2.contains(album)) {
			itemAlbum = _albums2.value(album);
		} else {
			itemAlbum = new LibraryItemAlbum(album);
			itemAlbum->setYear(year);
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
		if (_artistsAlbums.contains(theArtist + album)) {
			itemArtist = _artistsAlbums.value(theArtist + album);
		} else {
			itemArtist = new LibraryItemArtist(theArtist + " – " + album);
			_artistsAlbums.insert(theArtist + album, itemArtist);
			invisibleRootItem()->appendRow(itemArtist);
			this->insertLetter(theArtist);
		}
		// Level 2
		if (artistAlbum.isEmpty()) {
			itemTrack = new LibraryItemTrack(title, -1);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")", -1);
		}
		itemArtist->appendRow(itemTrack);
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
		if (_artistsAlbums.contains(theArtist + album)) {
			itemArtist = _artistsAlbums.value(theArtist + album);
		} else {
			itemArtist = new LibraryItemArtist(theArtist + " – " + album);
			_artistsAlbums.insert(theArtist + album, itemArtist);
			itemYear->appendRow(itemArtist);
		}
		// Level 3
		if (artistAlbum.isEmpty()) {
			itemTrack = new LibraryItemTrack(title, -1);
		} else {
			itemTrack = new LibraryItemTrack(title + " (" + artist + ")", -1);
		}
		itemArtist->appendRow(itemTrack);
		break;
	}
	itemTrack->setAlbum(album);
	itemTrack->setArtist(artist);
	itemTrack->setArtistAlbum(artistAlbum);
	itemTrack->setFilePath(absFilePath);
	itemTrack->setTrackNumber(trackNumber);
	itemTrack->setYear(year);
	_tracks.insert(itemTrack);
}

/// Strategy or Policy? Strategies are usually called from other classes
void LibraryModel::insertTrackFromFileSystem(const QString &absFilePath, const FileHelper &fileHelper)
{
	Tag *tag = fileHelper.file()->tag();
	QString artist = QString(tag->artist().toCString(true)).trimmed();
	QString artistAlbum = fileHelper.artistAlbum();
	QString album = QString(tag->album().toCString(true)).trimmed();
	QString title = QString(tag->title().toCString(true)).trimmed();
	this->insertTrack(absFilePath, artist, artistAlbum, album, title, tag->track(), tag->year());
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

		QByteArray output;
		QDataStream dataStream(&output, QIODevice::ReadWrite);
		QSetIterator<LibraryItemTrack*> it(_tracks);
		while (it.hasNext()) {
			it.next()->write(dataStream);
		}
		mmmmp.write(qCompress(output, 9));
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
