#include "librarytreeview.h"
#include "libraryitemdelegate.h"
#include "settings.h"
#include "libraryitem.h"

#include <QDirIterator>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>

#include <fileref.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>

#include <QtDebug>

using namespace TagLib;

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	QTreeView(parent)
{
	libraryModel = new LibraryModel(this);
	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	proxyModel->setSourceModel(libraryModel);

	Settings *settings = Settings::getInstance();

	this->setModel(proxyModel);
	this->setStyleSheet(settings->styleSheet(this));

	int iconSize = Settings::getInstance()->coverSize();
	this->setIconSize(QSize(iconSize, iconSize));
	parent->setWindowOpacity(0.5);

	//QWidget *centeredOverlay = new QWidget(this);
	//centeredOverlay->setBaseSize(this->width(), this->height());
	//centeredOverlay->setPalette(QPalette(QPalette::WindowText));
	//centeredOverlay->setAttribute(Qt::WA_NoSystemBackground, false);
	//centeredOverlay->setBackgroundRole(QPalette::Window);
	//circleProgressBar = new CircleProgressBar(centeredOverlay);

	circleProgressBar = new CircleProgressBar(this);
	musicSearchEngine = new MusicSearchEngine(this);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(beforeSendToPlaylist(const QModelIndex &)));
	connect(musicSearchEngine, SIGNAL(scannedCover(QString)), libraryModel, SLOT(addCoverPathToAlbum(QString)));
	connect(musicSearchEngine, SIGNAL(scannedFile(int, QString)), this, SLOT(readFile(int, QString)));
	connect(musicSearchEngine, SIGNAL(progressChanged(const int &)), circleProgressBar, SLOT(setValue(const int &)));

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(musicSearchEngine, SIGNAL(finished()), this, SLOT(endPopulateTree()));
	connect(libraryModel, SIGNAL(loadedFromFile()), this, SLOT(endPopulateTree()));

	// Tell the view to create specific delegate for the current row
	connect(libraryModel, SIGNAL(associateNodeWithDelegate(LibraryItem*)), this, SLOT(addNodeToTree(LibraryItem*)));

	connect(this, SIGNAL(sizeOfCoversChanged(int)), this, SLOT(setCoverSize(int)));

	// TEST : this widget is not repainted when font is changing, only when closing the Dialog :(
	connect(settings, SIGNAL(currentFontChanged()), this, SLOT(repaint()));

	// When the scan is complete, save the model in the filesystem
	connect(musicSearchEngine, SIGNAL(finished()), libraryModel, SLOT(saveToFile()));

	// Load covers only when an item need to be expanded
	connect(this, SIGNAL(expanded(QModelIndex)), proxyModel, SLOT(loadCovers(QModelIndex)));
}

/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
void LibraryTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
	// Save the position of the mouse, to be able to choose the correct action :
	// - add an item to the playlist
	// - edit stars to the current track
	currentPos = event->pos();
	QTreeView::mouseDoubleClickEvent(event);
}

/// Move to libraryModel?
void LibraryTreeView::readFile(int musicLocationIndex, const QString &qFileName)
{
	static LibraryItem *indexArtist = NULL;
	static LibraryItem *indexAlbum = NULL;
	Settings *settings = Settings::getInstance();
	settings->musicLocations().at(musicLocationIndex).toString();
	QString filePath = settings->musicLocations().at(musicLocationIndex).toString() + qFileName;
	MPEG::File fileRef(filePath.toLocal8Bit().data(), true, AudioProperties::Average);
	if (fileRef.isValid() && fileRef.tag()) {
		// For albums with multiple Artists, like OST, the "TPE2" value is commonly used for the tag "Album Artist"
		// It is used in Windows 7, foobar2000, etc
		ID3v2::Tag *tag = fileRef.ID3v2Tag();
		String artist;
		if (tag) {
			ID3v2::FrameList l = tag->frameListMap()["TPE2"];
			if (l.isEmpty()) {
				artist = fileRef.tag()->artist();
			} else {
				artist = l.front()->toString();
			}
		} else {
			artist = fileRef.tag()->artist();
		}

		// Is there is already this artist in the library?
		indexArtist = libraryModel->hasArtist(QString(artist.toCString(false)));
		if (indexArtist == NULL) {
			indexArtist = libraryModel->insertArtist(QString(artist.toCString(false)));
			LibraryItemDelegate *libraryItemDelegate = new LibraryItemDelegate(this);
			indexArtist->setDelegate(libraryItemDelegate);
			setItemDelegateForRow(indexArtist->row(), libraryItemDelegate);
		}

		// Is there is already an album from this artist?
		indexAlbum = libraryModel->hasAlbum(indexArtist, QString(fileRef.tag()->album().toCString(false)));
		if (indexAlbum == NULL) {
			// New album to create, only if it's not empty
			if (fileRef.tag()->album().isEmpty()) {
				indexAlbum = indexArtist;
			} else {
				indexAlbum = libraryModel->insertAlbum(QString(fileRef.tag()->album().toCString(false)), filePath, indexArtist);
				LibraryItemDelegate *libraryItemDelegate = new LibraryItemDelegate(this);
				indexAlbum->setDelegate(libraryItemDelegate);
				setItemDelegateForRow(indexAlbum->row(), libraryItemDelegate);
			}
		}

		// In every case, insert a new track
		QString title(fileRef.tag()->title().toCString(false));
		if (title.isEmpty()) {
			title = qFileName.left(qFileName.size() - 4); // 4 == ".mp3"
			title = title.mid(title.lastIndexOf('/')+1);
		}
		LibraryItem *track = libraryModel->insertTrack(musicLocationIndex, qFileName, fileRef.tag()->track(), title, indexAlbum);
		if (track) {
			LibraryItemDelegate *libraryItemDelegate = new LibraryItemDelegate(this);
			track->setDelegate(libraryItemDelegate);
			setItemDelegateForRow(track->row(), libraryItemDelegate);
		}
	}
}

/** Tell the view to create specific delegate for the current row. */
void LibraryTreeView::addNodeToTree(LibraryItem *libraryItem)
{
	LibraryItemDelegate *libraryItemDelegate = new LibraryItemDelegate(this);
	libraryItem->setDelegate(libraryItemDelegate);
	setItemDelegateForRow(libraryItem->row(), libraryItemDelegate);
}

/** Check if the current double-clicked item is an Artist, an Album or a Track.*/
void LibraryTreeView::beforeSendToPlaylist(const QModelIndex &index)
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegateForRow(index.row()));
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		QStandardItem *item = libraryModel->itemFromIndex(sourceIndex);
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				beforeSendToPlaylist(index.child(i, 0));
			}
		} else if (item->data(LibraryItem::MEDIA_TYPE).toInt() != LibraryModel::LETTER) {
			// If the click from the mouse was on a text label or on a star
			if (!Settings::getInstance()->isStarDelegates() ||
					(delegate->title()->contains(currentPos) || (delegate->title()->isEmpty() && delegate->stars()->isEmpty()))) {
				emit sendToPlaylist(QPersistentModelIndex(sourceIndex));
			} else if (delegate->stars()->contains(currentPos)) {
				QStyleOptionViewItemV4 qsovi;
				QWidget *editor = delegate->createEditor(this, qsovi, sourceIndex);
				editor->show();
			}
		}
	}
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (!filter.isEmpty()) {
		bool needToSortAgain = false;
		if (proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
			collapseAll();
			sortByColumn(0, Qt::AscendingOrder);
		}
	} else {
		proxyModel->setFilterRegExp(QRegExp());
		collapseAll();
		sortByColumn(0, Qt::AscendingOrder);
	}
}

/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
void LibraryTreeView::beginPopulateTree(bool musicLocationHasChanged)
{
	circleProgressBar->show();
	// Clean all before scanning again
	// Seems difficult to clean efficiently delegates: they are disabled right now.
	libraryModel->clear();
	this->reset();
	if (musicLocationHasChanged) {
		musicSearchEngine->start();
	} else {
		if (QFile::exists("library.mmmmp")) {
			libraryModel->loadFromFile();
		} else {
			// If the file has been erased from the disk meanwhile
			this->beginPopulateTree(true);
		}
	}
}

void LibraryTreeView::endPopulateTree()
{
	sortByColumn(0, Qt::AscendingOrder);
	circleProgressBar->hide();
	circleProgressBar->setValue(0);
}

void LibraryTreeView::expandTreeView(const QModelIndex &index)
{
	QModelIndex parent = index;
	while (parent.parent().isValid()) {
		expand(parent);
		parent = parent.parent();
	}
	expand(parent);
}

/**  Layout the library at runtime when one is changing the size in options. */
void LibraryTreeView::setCoverSize(int newSize)
{
	Settings *settings = Settings::getInstance();
	int oldSize = settings->coverSize();
	int bufferedCoverSize = settings->bufferedCoverSize();
	settings->setCoverSize(newSize);

	bool coversNeedToBeReloaded = true;

	// Increase buffer or not
	static const short buffer = 128;
	if (newSize < bufferedCoverSize) {
		if (newSize + buffer < bufferedCoverSize) {
			bufferedCoverSize -= buffer;
			settings->setBufferedCoverSize(bufferedCoverSize);
		} else {
			coversNeedToBeReloaded = false;
		}
	} else if (oldSize <= newSize) {
		bufferedCoverSize += buffer;
		settings->setBufferedCoverSize(bufferedCoverSize);
	}

	// Scales covers for every expanded item in the tree
	if (coversNeedToBeReloaded) {
		for (int i=0; i<proxyModel->rowCount(rootIndex()); i++) {
			QModelIndex index = proxyModel->index(i, 0);
			// It's really slow to reload covers from disk for every changes in the User Interface.
			// It's more efficient to load icons just for some resolutions, like 128x128 or 512x512.
			if (isExpanded(index)) {
				proxyModel->loadCovers(index);
			}
		}
	}

	// Upscale (or downscale) icons because their inner representation is already greater than what's displayed
	this->setIconSize(QSize(newSize, newSize));
}
