#include "librarymodel.h"

#include <QDir>
#include <QStandardPaths>
#include <QThread>

#include <QtDebug>

#include "musicsearchengine.h"

using namespace TagLib;

LibraryModel::LibraryModel(QObject *parent)
	 : QStandardItemModel(0, 1, parent)
{
	MusicSearchEngine *musicSearchEngine = new MusicSearchEngine();
	QThread *worker = new QThread();
	//connect(this, &LibraryTreeView::searchMusic, musicSearchEngine, &MusicSearchEngine::doSearch);
	connect(musicSearchEngine, &MusicSearchEngine::scannedCover, this, &LibraryModel::addCoverPathToAlbum);
	connect(musicSearchEngine, &MusicSearchEngine::scannedFiled, this, &LibraryModel::readFile, Qt::BlockingQueuedConnection);
	connect(musicSearchEngine, &MusicSearchEngine::progressChanged, this, &LibraryModel::progressChanged);
	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, &LibraryModel::searchHasEnded);

	// When the scan is complete, save the model in the filesystem
	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, &LibraryModel::saveToFile);

	musicSearchEngine->moveToThread(worker);
	worker->start();
}

/** Removes everything. */
void LibraryModel::clear()
{
	/*_albums.clear();
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
	}*/
}

/** Add (a path to) an icon to every album. */
void LibraryModel::addCoverPathToAlbum(const QString &fileName)
{
	QFileInfo fileInfo(fileName);
	//LibraryItemAlbum *album = _albumsAbsPath.value(fileInfo.absolutePath());
	LibraryItemAlbum *album = NULL;
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
		//PersistentItem persistedItem;
		while (!inputDataStream.atEnd()) {
			PersistentItem *item = new PersistentItem();
			item->read(inputDataStream);
			_persistentItems.insert(item);
			qDebug() << "about to emit trackCreated(item)";
			emit trackCreated(item);
			//qDebug() << "apres" << item->absoluteFilePath();
			/*this->insertTrack(persistedItem.absoluteFilePath(), persistedItem.artist(), persistedItem.artistAlbum(), persistedItem.album(),
							  persistedItem.text(), persistedItem.trackNumber(), persistedItem.discNumber(), persistedItem.year());
			if (!persistedItem.coverFileName().isEmpty()) {
				//qDebug() << "cover is not empty. adding from file " << persistedItem.absoluteFilePath();
				this->addCoverPathToAlbum(persistedItem.absolutePath() + '/' + persistedItem.coverFileName());
			} else {
				//qDebug() << "cover is empty. skipping file" << persistedItem.absoluteFilePath();
			}*/
		}
		mmmmp.close();
		emit loadedFromFile();
	}
}

/** Read a file from the filesystem and adds it into the library. */
void LibraryModel::readFile(const QString &absFilePath)
{
	FileHelper fh(absFilePath);
	/// XXX already created in fh
	QFileInfo fileInfo(absFilePath);
	//if (fh.file() != NULL && fh.file()->tag() != NULL && !fh.file()->tag()->isEmpty()) {
	if (fh.isValid()) {
		PersistentItem *item = new PersistentItem(new LibraryItemTrack(fh.title()));
		///
		item->setAbsoluteFilePath(absFilePath, fileInfo.fileName());
		item->setAlbum(fh.album());
		item->setArtist(fh.artist());
		item->setArtistAlbum(fh.artistAlbum());
		item->setDiscNumber(fh.discNumber());
		item->setTrackNumber(fh.trackNumber().toInt());
		item->setYear(fh.year().toInt());
		_persistentItems.insert(item);
		emit trackCreated(item);
	} else {
		qDebug() << "fh.isValid() is false for:" << absFilePath;
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
