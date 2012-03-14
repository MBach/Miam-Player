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
	//proxyModel->setFilterRole(Qt::UserRole+1);

	//proxyModel->setHeaderData(0, Qt::Horizontal, QVariant("Artists"));
	QStringList labels("Artists");
	libraryModel->setHorizontalHeaderLabels(labels);
	this->setModel(proxyModel);

	//QHeaderView *header = new QHeaderView(Qt::Horizontal, this);
	this->setIconSize(QSize(48, 48));
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
	connect(musicSearchEngine, SIGNAL(scannedCover(QString)), this, SLOT(readCover(QString)));
	connect(musicSearchEngine, SIGNAL(scannedFile(int, QString)), this, SLOT(readFile(int, QString)));
	connect(musicSearchEngine, SIGNAL(progressChanged(const int &)), circleProgressBar, SLOT(setValue(const int &)));
	connect(musicSearchEngine, SIGNAL(finished()), this, SLOT(endPopulateTree()));

	connect(libraryModel, SIGNAL(loadedFromFile()), this, SLOT(endPopulateTree()));
	// Tell the view to create specific delegate for the current row
	connect(libraryModel, SIGNAL(associateNodeWithDelegate(int)), this, SLOT(addNodeToTree(int)));

	// Forward this signal to the inner model which quickly activates/desactivates icons
	connect(this, SIGNAL(setIcon(bool)), libraryModel, SLOT(setIcon(bool)));

	// TEST : this widget is not repainted when font is changing, only when closing the Dialog :(
	Settings *settings = Settings::getInstance();
	connect(settings, SIGNAL(currentFontChanged()), this, SLOT(repaint()));

	// When the scan is complete, save the model in the filesystem
	connect(musicSearchEngine, SIGNAL(finished()), libraryModel, SLOT(saveToFile()));

	//TEST
	connect(proxyModel, SIGNAL(aboutToExpand(QModelIndex)), this, SLOT(expandTreeView(QModelIndex)));
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
		}

		// Is there is already an album from this artist?
		indexAlbum = libraryModel->hasAlbum(QString(fileRef.tag()->album().toCString(false)));
		if (indexAlbum == NULL) {
			// New album to create, only if it's not empty
			if (fileRef.tag()->album().isEmpty()) {
				indexAlbum = indexArtist;
			} else {
				indexAlbum = libraryModel->insertAlbum(QString(fileRef.tag()->album().toCString(false)), filePath, indexArtist);
			}
		}

		// In every case, insert a new track
		QString title(fileRef.tag()->title().toCString(false));
		LibraryItem *track = libraryModel->insertTrack(musicLocationIndex, qFileName, fileRef.tag()->track(), title, indexAlbum);
		setItemDelegateForRow(track->row(), new LibraryItemDelegate(this));
	}
}

void LibraryTreeView::readCover(const QString &qFileName)
{
	LibraryItem *indexAlbum = libraryModel->hasCover(qFileName.left(qFileName.lastIndexOf('/')));
	if (indexAlbum) {
		QIcon icon;
		QSize size(48, 48);
		icon.addFile(qFileName, size);
		libraryModel->insertAlbumIcon(icon, indexAlbum);
	}
}

/** Tell the view to create specific delegate for the current row. */
void LibraryTreeView::addNodeToTree(int row)
{
	setItemDelegateForRow(row, new LibraryItemDelegate(this));
}

/** Check if the current double-clicked item is an Artist, an Album or a Track.*/
void LibraryTreeView::beforeSendToPlaylist(const QModelIndex &index)
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegateForRow(index.row()));
	if (delegate) {
		// If the click from the mouse was on a text label or on a star
		if (delegate->title()->contains(currentPos)) {
			QStandardItem *item = libraryModel->itemFromIndex(proxyModel->mapToSource(index));
			if (item != NULL) {
				if (item->hasChildren()) {
					for (int i=0; i < item->rowCount(); i++) {
						//recursive call on children
						beforeSendToPlaylist(index.child(i, 0));
					}
				} else {
					emit sendToPlaylist(QPersistentModelIndex(index));
				}
			}
		} else if (delegate->stars()->contains(currentPos)) {
			QStyleOptionViewItem qsovi;
			QWidget *editor = delegate->createEditor(this, qsovi, index);
			editor->show();
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
		/*if (filter.length() == 1) {
			//expandToDepth(0);
		} else {
			//expandAll();
		}*/
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
	libraryModel->clear();
	if (musicLocationHasChanged) {
		musicSearchEngine->start();
	} else {
		QFile mmmmp("library.mmmmp");
		if (mmmmp.open(QIODevice::ReadOnly)) {
			mmmmp.close();
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

	//libraryModel->
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
