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
#include <QStandardPaths>
#include <QThread>

#include <QtDebug>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent)
{
	libraryModel = new LibraryModel(this);
	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSourceModel(libraryModel);

	Settings *settings = Settings::getInstance();

	this->setModel(proxyModel);
	this->setStyleSheet(settings->styleSheet(this));
	this->header()->setStyleSheet(settings->styleSheet(this->header()));
	this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));

	int iconSize = Settings::getInstance()->coverSize();
	this->setIconSize(QSize(iconSize, iconSize));
	this->setItemDelegate(new LibraryItemDelegate(this));

	proxyModel->setHeaderData(0, Qt::Horizontal, tr("  Artists \\ Albums"), Qt::DisplayRole);
	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(Settings::MENUS), Qt::FontRole);

	//QHBoxLayout *vLayout = new QHBoxLayout();
	//this->setLayout(vLayout);
	//QWidget *centeredOverlay = new QWidget(this);
	//qDebug() << this->width() << this->height();
	//centeredOverlay->setBaseSize(this->width(), this->height());
	//QPalette pal;
	//pal.setColor(QPalette::Window, Qt::black);
	//centeredOverlay->setPalette(pal);
	//centeredOverlay->setAttribute(Qt::WA_NoSystemBackground, false);
	//circleProgressBar = new CircleProgressBar(centeredOverlay);
	//QGraphicsOpacityEffect *goe = new QGraphicsOpacityEffect(circleProgressBar);
	//goe->setOpacity(0.5);
	//circleProgressBar->setGraphicsEffect(goe);

	circleProgressBar = new CircleProgressBar(this);
	circleProgressBar->setTransparentCenter(true);

	QThread *worker = new QThread();
	MusicSearchEngine *musicSearchEngine = new MusicSearchEngine();
	musicSearchEngine->moveToThread(worker);
	worker->start();
	connect(this, &LibraryTreeView::searchMusic, musicSearchEngine, &MusicSearchEngine::doSearch);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	//QAction *actionOpenStarEditor = new QAction(tr("Ratings: *****"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Properties"), this);
	properties = new QMenu(this);
	properties->addAction(actionSendToCurrentPlaylist);
	//properties->addAction(actionOpenStarEditor);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	connect(this, &QTreeView::doubleClicked, [=] (const QModelIndex &) { appendToPlaylist(); });
	connect(musicSearchEngine, &MusicSearchEngine::scannedCover, libraryModel, &LibraryModel::addCoverPathToAlbum);
	connect(musicSearchEngine, &MusicSearchEngine::scannedFile, libraryModel, &LibraryModel::readFile);
	connect(musicSearchEngine, &MusicSearchEngine::progressChanged, circleProgressBar, &QProgressBar::setValue);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(musicSearchEngine, &MusicSearchEngine::endSearch, this, &LibraryTreeView::endPopulateTree);
	connect(libraryModel, &LibraryModel::loadedFromFile, this, &LibraryTreeView::endPopulateTree);

	// When the scan is complete, save the model in the filesystem
	connect(musicSearchEngine, &MusicSearchEngine::endSearch, libraryModel, &LibraryModel::saveToFile);

	// Load covers only when an item need to be expanded
	connect(this, &QTreeView::expanded, proxyModel, &LibraryFilterProxyModel::loadCovers);

	// Context menu
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
    connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
}

/** Small function for translating the QMenu exclusively. */
void LibraryTreeView::retranslateUi()
{
	foreach (QAction *action, properties->actions()) {
		action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
	}
}

QSize LibraryTreeView::sizeInt() const
{
	return QSize();
}


/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		LibraryItem *libraryItem = static_cast<LibraryItem*>(item);
		if (!(libraryItem && libraryItem->type() == LibraryModel::LETTER)) {
			properties->exec(event->globalPos());
		}
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

/** Recursive count for leaves only. */
int LibraryTreeView::count(const QModelIndex &index) const
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegate());
	if (delegate) {
		QStandardItem *item = libraryModel->itemFromIndex(proxyModel->mapToSource(index));
		int tmp = 0;
		for (int i = 0; i < item->rowCount(); i++) {
			tmp += count(index.child(i, 0));
		}
		return (tmp == 0) ? 1 : tmp;
	}
	return 0;
}

/** Reimplemented. */
int LibraryTreeView::countAll(const QModelIndexList &indexes) const
{
	int c = 0;
	foreach (QModelIndex index, indexes) {
		c += this->count(index);
	}
	return c;
}

/** Reimplemented. */
void LibraryTreeView::findAll(const QPersistentModelIndex &index, QMap<QString, QPersistentModelIndex> &indexes)
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegate());
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		QStandardItem *item = libraryModel->itemFromIndex(sourceIndex);
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), indexes);
			}
		} else if (item->data(LibraryItem::MEDIA_TYPE).toInt() != LibraryModel::LETTER) {
			// If the click from the mouse was on a text label or on a star
			if (!Settings::getInstance()->isStarDelegates() ||
					(delegate->title()->contains(currentPos) || (delegate->title()->isEmpty() && delegate->stars()->isEmpty()))) {
				indexes.insert(TreeView::absFilePath(index), sourceIndex);
			} else if (delegate->stars()->contains(currentPos)) {
				QStyleOptionViewItemV4 qsovi;
				QWidget *editor = delegate->createEditor(this, qsovi, sourceIndex);
				editor->show();
			}
		}
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

		emit searchMusic();

	} else {
		if (QFile::exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"))) {
			libraryModel->loadFromFile();
		} else {
			// If the file has been erased from the disk meanwhile
			this->beginPopulateTree(true);
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

/** Rebuild a subset of the tree. */
void LibraryTreeView::rebuild(QList<QPersistentModelIndex> indexes)
{
	// Parse once again those items
	foreach (QPersistentModelIndex index, indexes) {
		QStandardItem *item = libraryModel->itemFromIndex(index);
		if (item) {
			LibraryItem *libraryItem = static_cast<LibraryItem*>(item);
			if (libraryItem) {
				int i = libraryItem->data(LibraryItem::IDX_TO_ABS_PATH).toInt();
				QString file = libraryItem->data(LibraryItem::REL_PATH_TO_MEDIA).toString();
				libraryModel->readFile(i, file);
			}
		}
	}
	// Remove items that were tagged as modified
	foreach (QPersistentModelIndex index, indexes) {
		libraryModel->removeNode(index);
	}
	libraryModel->makeSeparators();
	sortByColumn(0, Qt::AscendingOrder);
	libraryModel->saveToFile();
}

void LibraryTreeView::endPopulateTree()
{
	if (Settings::getInstance()->toggleSeparators()) {
		libraryModel->makeSeparators();
	}
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
