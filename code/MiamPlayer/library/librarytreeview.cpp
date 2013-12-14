#include "librarytreeview.h"
#include "settings.h"

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

#include <filehelper.h>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new QStandardItemModel(parent)), sqlModel(NULL)
{
	Settings *settings = Settings::getInstance();

	_libraryModel->setColumnCount(1);
	_libraryModel->setHorizontalHeaderItem(0, new QStandardItem(tr("  Artists \\ Albums")));
	this->setModel(_libraryModel);

	///FIXME
	//this->setStyleSheet(settings->styleSheet(this));
	//this->header()->setStyleSheet(settings->styleSheet(this->header()));
	//this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));

	int iconSize = settings->coverSize();
	this->setIconSize(QSize(iconSize, iconSize));

	this->header()->setContextMenuPolicy(Qt::CustomContextMenu);

	proxyModel = new LibraryFilterProxyModel(this);

	circleProgressBar = new CircleProgressBar(this);
	circleProgressBar->setTransparentCenter(true);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Properties"), this);
	properties = new QMenu(this);
	properties->addAction(actionSendToCurrentPlaylist);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	connect(this, &QTreeView::doubleClicked, [=] (const QModelIndex &) { appendToPlaylist(); });

	// Context menu
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
    connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);

	sortByColumn(0, Qt::AscendingOrder);
}

void LibraryTreeView::init(LibrarySqlModel *sql)
{
	sqlModel = sql;

	Settings *settings = Settings::getInstance();

	proxyModel->setSourceModel(_libraryModel);
	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(Settings::MENUS), Qt::FontRole);

	this->setItemDelegate(new LibraryItemDelegate(proxyModel));
	//connect(_libraryModel, &LibraryModel::progressChanged, circleProgressBar, &QProgressBar::setValue);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(sqlModel, &LibrarySqlModel::modelAboutToBeReset, this, &LibraryTreeView::reset);
	connect(sqlModel, &LibrarySqlModel::trackCreated, this, &LibraryTreeView::insertTrack);
	connect(sqlModel, &LibrarySqlModel::modelReset, this, &LibraryTreeView::endPopulateTree);

	// One can choose a hierarchical order for drawing the library
	LibraryOrderDialog *_lod = new LibraryOrderDialog(this);
	connect(header(), &QHeaderView::customContextMenuRequested, [=](const QPoint &pos) {
		_lod->move(mapToGlobal(pos));
		_lod->show();
	});
	connect(_lod, &LibraryOrderDialog::aboutToRedrawLibrary, sqlModel, &LibrarySqlModel::loadFromFileDB);
}

void LibraryTreeView::insertLetter(const QString &letters)
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
			_libraryModel->invisibleRootItem()->appendRow(new QStandardItem(letter));
		}
	}
}

void LibraryTreeView::insertTrack(const FileHelper &fh)
{
	QString theArtist = fh.artistAlbum().isEmpty() ? fh.artist() : fh.artistAlbum();

	QStandardItem *itemArtist = NULL;
	QStandardItem *itemAlbum = NULL;
	QStandardItem *itemTrack = NULL;
	QStandardItem *itemYear = NULL;
	QStandardItem *itemDiscNumber = NULL;

	//qDebug() << "about to insert" << fh.title();

	static bool existingArtist = true;
	switch (Settings::getInstance()->value("insertPolicy").toInt()) {
	case Artist:
		// Level 1
		if (_artists.contains(theArtist.toLower())) {
			itemArtist = _artists.value(theArtist.toLower());
			existingArtist = true;
		} else {
			itemArtist = new QStandardItem(theArtist);
			itemArtist->setData(Qt::UserRole + 1);
			_artists.insert(theArtist.toLower(), itemArtist);
			_libraryModel->invisibleRootItem()->appendRow(itemArtist);
			this->insertLetter(theArtist);
			existingArtist = false;
		}
		// Level 2
		if (existingArtist && _albums.contains(QPair<QStandardItem*, QString>(itemArtist, fh.album()))) {
			itemAlbum = _albums.value(QPair<QStandardItem*, QString>(itemArtist, fh.album()));
		} else {
			itemAlbum = new LibraryItem(fh.album());
			_albums.insert(QPair<QStandardItem *, QString>(itemArtist, fh.album()), itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3 (option)
		if (fh.discNumber() > 0 && !_discNumbers.contains(QPair<QStandardItem*, int>(itemAlbum, fh.discNumber()))) {
			itemDiscNumber = new QStandardItem(QString::number(fh.discNumber()));
			_discNumbers.insert(QPair<QStandardItem *, int>(itemAlbum, fh.discNumber()), itemDiscNumber);
			itemAlbum->appendRow(itemDiscNumber);
		}
		// Level 3
		if (fh.artistAlbum().isEmpty() || QString::compare(fh.artist(), fh.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(fh.title());
		} else {
			itemTrack = new QStandardItem(fh.title() + " (" + fh.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Album:
		// Level 1
		if (_albums2.contains(fh.album())) {
			itemAlbum = _albums2.value(fh.album());
		} else {
			itemAlbum = new QStandardItem(fh.album());
			_albums2.insert(fh.album(), itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(fh.album());
		}
		// Level 2
		itemTrack = new QStandardItem(fh.title());
		itemAlbum->appendRow(itemTrack);
		break;
	case ArtistAlbum:
		// Level 1
		if (_albums2.contains(theArtist + fh.album())) {
			itemAlbum = _albums2.value(theArtist + fh.album());
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + fh.album());
			_albums2.insert(theArtist + fh.album(), itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(theArtist);
		}
		// Level 2
		if (fh.artistAlbum().isEmpty() || QString::compare(fh.artist(), fh.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(fh.title());
		} else {
			itemTrack = new QStandardItem(fh.title() + " (" + fh.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Year:
		// Level 1
		if (_years.contains(fh.year())) {
			itemYear = _years.value(fh.year());
		} else {
			if (fh.year().toInt() > 0) {
				itemYear = new QStandardItem(fh.year());
			} else {
				itemYear = new QStandardItem();
			}
			_years.insert(fh.year(), itemYear);
			_libraryModel->invisibleRootItem()->appendRow(itemYear);
		}
		// Level 2
		if (_albums2.contains(theArtist + fh.album())) {
			itemAlbum = _albums2.value(theArtist + fh.album());
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + fh.album());
			_albums2.insert(theArtist + fh.album(), itemAlbum);
			itemYear->appendRow(itemAlbum);
		}
		// Level 3
		if (fh.artistAlbum().isEmpty() || QString::compare(fh.artist(), fh.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(fh.title());
		} else {
			itemTrack = new QStandardItem(fh.title() + " (" + fh.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	}
	itemTrack->setData(fh.absFilePath(), AbsPath);
}


/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		foreach (QAction *action, properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
		}
		/*if (item->type() != LibraryItem::Letter) {
			properties->exec(event->globalPos());
		}*/
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
		QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(index));
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
	/*LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegate());
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		//LibraryItem *item = _libraryModel->itemFromIndex(sourceIndex);
		QStandardItem *item = _libraryModel->itemFromIndex(sourceIndex);
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), tracks);
			}
		} else if (item->type() == LibraryItem::Track) {
			LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
			tracks.append(track->absoluteFilePath());
		}
	}*/
}

/** Reimplemented. */
void LibraryTreeView::reset()
{
	//circleProgressBar->show();
	if (_libraryModel->rowCount() > 0) {
		qDebug() << Q_FUNC_INFO;
		_artists.clear();
		_albums.clear();
		_discNumbers.clear();
		_albums2.clear();
		_albumsAbsPath.clear();
		_artistsAlbums.clear();
		_years.clear();
		_letters.clear();
		_libraryModel->removeRows(0, _libraryModel->rowCount());
	}
	switch (Settings::getInstance()->value("insertPolicy").toInt()) {
	case Artist:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists \\ Albums"));
		break;
	case Album:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Albums"));
		break;
	case ArtistAlbum:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists – Albums"));
		break;
	case Year:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Years"));
		break;
	}
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (filter.isEmpty()) {
		proxyModel->setFilterRegExp(QRegExp());
		collapseAll();
		sortByColumn(0, Qt::AscendingOrder);
	} else {
		bool needToSortAgain = false;
		if (proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
			collapseAll();
			sortByColumn(0, Qt::AscendingOrder);
		}
	}
}

void LibraryTreeView::endPopulateTree()
{
	qDebug() << Q_FUNC_INFO;
	sortByColumn(0, Qt::AscendingOrder);
	circleProgressBar->hide();
	circleProgressBar->setValue(0);
}
