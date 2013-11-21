#include "librarytreeview.h"
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

#include "library/libraryitemdelegate.h"
#include "library/libraryorderdialog.h"

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent)
{
	libraryModel = new LibraryModel(this);
	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSourceModel(libraryModel);

	Settings *settings = Settings::getInstance();

	this->setModel(proxyModel);
	///FIXME
	//this->setStyleSheet(settings->styleSheet(this));
	//this->header()->setStyleSheet(settings->styleSheet(this->header()));
	//this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));

	int iconSize = settings->coverSize();
	this->setIconSize(QSize(iconSize, iconSize));

	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(Settings::MENUS), Qt::FontRole);

	this->setItemDelegate(new LibraryItemDelegate(proxyModel));

	this->header()->setContextMenuPolicy(Qt::CustomContextMenu);

	// One can choose a hierarchical order for drawing the library
	LibraryOrderDialog *lod = new LibraryOrderDialog(this);
	lod->setModel(libraryModel);
	connect(header(), &QHeaderView::customContextMenuRequested, [=](const QPoint &pos) {
		lod->move(mapToGlobal(pos));
		lod->show();
	});
	connect(lod, &LibraryOrderDialog::aboutToRedrawLibrary, this, &LibraryTreeView::beginPopulateTree);

	circleProgressBar = new CircleProgressBar(this);
	circleProgressBar->setTransparentCenter(true);

	QThread *worker = new QThread();
	MusicSearchEngine *musicSearchEngine = new MusicSearchEngine();
	musicSearchEngine->moveToThread(worker);
	worker->start();
	connect(this, &LibraryTreeView::searchMusic, musicSearchEngine, &MusicSearchEngine::doSearch);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Properties"), this);
	properties = new QMenu(this);
	properties->addAction(actionSendToCurrentPlaylist);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	connect(this, &QTreeView::doubleClicked, [=] (const QModelIndex &) { appendToPlaylist(); });
	connect(musicSearchEngine, &MusicSearchEngine::scannedCover, libraryModel, &LibraryModel::addCoverPathToAlbum);
	connect(musicSearchEngine, &MusicSearchEngine::scannedFiled, libraryModel, &LibraryModel::readFile, Qt::BlockingQueuedConnection);

	connect(musicSearchEngine, &MusicSearchEngine::progressChanged, circleProgressBar, &QProgressBar::setValue);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, &LibraryTreeView::endPopulateTree);
	connect(libraryModel, &LibraryModel::loadedFromFile, this, &LibraryTreeView::endPopulateTree);

	// When the scan is complete, save the model in the filesystem
	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, libraryModel, &LibraryModel::saveToFile);

	// Context menu
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
    connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);

	sortByColumn(0, Qt::AscendingOrder);
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	LibraryItem *item = libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		foreach (QAction *action, properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
		}
		if (item->type() != LibraryItem::Letter) {
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
void LibraryTreeView::findAll(const QPersistentModelIndex &index, QStringList &tracks)
{
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegate());
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		LibraryItem *item = libraryModel->itemFromIndex(sourceIndex);
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), tracks);
			}
		} else if (item->type() == LibraryItem::Track) {
			LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
			tracks.append(track->absoluteFilePath());
		}
	}
}

/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
void LibraryTreeView::beginPopulateTree(bool musicLocationHasChanged)
{
	circleProgressBar->show();
	libraryModel->clear();
	this->reset();
	switch (Settings::getInstance()->insertPolicy()) {
	case Settings::Artist:
		proxyModel->setHeaderData(0, Qt::Horizontal, tr("  Artists \\ Albums"), Qt::DisplayRole);
		break;
	case Settings::Album:
		proxyModel->setHeaderData(0, Qt::Horizontal, tr("  Albums"), Qt::DisplayRole);
		break;
	case Settings::ArtistAlbum:
		proxyModel->setHeaderData(0, Qt::Horizontal, tr("  Artists – Albums"), Qt::DisplayRole);
		break;
	case Settings::Year:
		proxyModel->setHeaderData(0, Qt::Horizontal, tr("  Years"), Qt::DisplayRole);
		break;
	}
	if (musicLocationHasChanged) {
		emit searchMusic();
	} else if (QFile::exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"))) {
		libraryModel->loadFromFile();
	} else {
		// If the file has been erased from the disk meanwhile
		this->beginPopulateTree(true);
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
		LibraryItem *item = libraryModel->itemFromIndex(index);
		if (item) {
			//int i = item->data(LibraryItem::IDX_TO_ABS_PATH).toInt();
			//QString file = item->data(LibraryItem::REL_PATH_TO_MEDIA).toString();
			//item->filePath();
			//libraryModel->readFile(file);
		}
	}
	// Remove items that were tagged as modified
	/// FIXME
	//foreach (QPersistentModelIndex index, indexes) {
		//libraryModel->removeNode(index);
	//}
	sortByColumn(0, Qt::AscendingOrder);
	//libraryModel->saveToFile();
}

void LibraryTreeView::endPopulateTree()
{
	sortByColumn(0, Qt::AscendingOrder);
	circleProgressBar->hide();
	circleProgressBar->setValue(0);
}
