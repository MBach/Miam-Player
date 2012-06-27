#include "librarytreeview.h"
#include "libraryitemdelegate.h"
#include "settings.h"
#include "libraryitem.h"

#include <QApplication>
#include <QDirIterator>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QScrollBar>

#include <QtDebug>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	QTreeView(parent)
{
	libraryModel = new LibraryModel(this);
	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	proxyModel->setSourceModel(libraryModel);

	Settings *settings = Settings::getInstance();

	this->setContextMenuPolicy(Qt::CustomContextMenu);
	this->setModel(proxyModel);
	this->setStyleSheet(settings->styleSheet(this));
	this->header()->setStyleSheet(settings->styleSheet(this->header()));
	this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));

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

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	//QAction *actionOpenStarEditor = new QAction(tr("Ratings: *****"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Properties"), this);
	properties = new QMenu(this);
	properties->addAction(actionSendToCurrentPlaylist);
	//properties->addAction(actionOpenStarEditor);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	connect(actionOpenTagEditor, SIGNAL(triggered()), this, SLOT(openTagEditor()));

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(findAllAndDispatch(const QModelIndex &)));
	connect(musicSearchEngine, SIGNAL(scannedCover(QString)), libraryModel, SLOT(addCoverPathToAlbum(QString)));
	connect(musicSearchEngine, SIGNAL(scannedFile(int, QString)), libraryModel, SLOT(readFile(int, QString)));
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

	// Context menu
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	connect(actionSendToCurrentPlaylist, SIGNAL(triggered()), this, SLOT(sendToCurrentPlaylist()));
}

/** Small function for translating the QMenu exclusively. */
void LibraryTreeView::retranslateUi()
{
	foreach (QAction *action, properties->actions()) {
		action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data(), 0, QApplication::UnicodeUTF8));
	}
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

/** Tell the view to create specific delegate for the current row. */
void LibraryTreeView::addNodeToTree(LibraryItem *libraryItem)
{
	LibraryItemDelegate *libraryItemDelegate = new LibraryItemDelegate(this);
	libraryItem->setDelegate(libraryItemDelegate);
	setItemDelegateForRow(libraryItem->row(), libraryItemDelegate);
}

/** Recursively scan one node and its subitems before dispatching tracks to a specific widget (playlist or tageditor).*/
void LibraryTreeView::findAllAndDispatch(const QModelIndex &index, bool toPlaylist)
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegateForRow(index.row()));
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		QStandardItem *item = libraryModel->itemFromIndex(sourceIndex);
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				findAllAndDispatch(index.child(i, 0), toPlaylist);
			}
		} else if (item->data(LibraryItem::MEDIA_TYPE).toInt() != LibraryModel::LETTER) {
			// If the click from the mouse was on a text label or on a star
			if (!Settings::getInstance()->isStarDelegates() ||
					(delegate->title()->contains(currentPos) || (delegate->title()->isEmpty() && delegate->stars()->isEmpty()))) {

				// Dispatch items
				if (toPlaylist) {
					emit sendToPlaylist(QPersistentModelIndex(sourceIndex));
				} else {
					emit sendToTagEditor(QPersistentModelIndex(sourceIndex));
				}
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

/// TEST
void LibraryTreeView::openTagEditor()
{
	/// Can those signals be factorised?
	emit setTagEditorVisible(true);
	emit aboutToBeSent();

	// Feed with new indexes
	QModelIndexList indexes = this->selectedIndexes();
	foreach (QModelIndex index, indexes) {
		this->findAllAndDispatch(index, false);
	}
	emit finishedToBeSent();
}

/// TEST
void LibraryTreeView::sendToCurrentPlaylist()
{
	/// FIXME: one can expand Artist \ Album \ tracks, select track #1 and album #1 and artist,
	/// so track #1 will be include 3 times, and album #1 2 times!
	/// But the previously written findAllAndDispatch recursively send items to the playlist, and does not filter
	foreach(QModelIndex index, selectedIndexes()) {
		this->findAllAndDispatch(index);
	}
}

/// TEST
void LibraryTreeView::showContextMenu(QPoint point)
{
	QModelIndex index = this->indexAt(point);
	QStandardItem *item = libraryModel->itemFromIndex(proxyModel->mapToSource(index));
	LibraryItem *libraryItem = static_cast<LibraryItem*>(item);
	if (!(libraryItem && libraryItem->type() == LibraryModel::LETTER)) {
		properties->exec(mapToGlobal(point));
	}
}
