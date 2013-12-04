#include "librarytreeview.h"
#include "settings.h"
#include <model/libraryitem.h>

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

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(NULL)
{
	Settings *settings = Settings::getInstance();
	_currentInsertPolicy = settings->insertPolicy();

	///FIXME
	//this->setStyleSheet(settings->styleSheet(this));
	//this->header()->setStyleSheet(settings->styleSheet(this->header()));
	//this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));

	int iconSize = settings->coverSize();
	this->setIconSize(QSize(iconSize, iconSize));

	this->header()->setContextMenuPolicy(Qt::CustomContextMenu);

	proxyModel = new LibraryFilterProxyModel(this);

	// One can choose a hierarchical order for drawing the library
	_lod = new LibraryOrderDialog(this);
	connect(header(), &QHeaderView::customContextMenuRequested, [=](const QPoint &pos) {
		_lod->move(mapToGlobal(pos));
		_lod->show();
	});
	connect(_lod, &LibraryOrderDialog::aboutToRedrawLibrary, this, &LibraryTreeView::beginPopulateTree);

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

void LibraryTreeView::init()
{
	Settings *settings = Settings::getInstance();

	proxyModel->setSourceModel(_libraryModel);
	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(Settings::MENUS), Qt::FontRole);
	_lod->setModel(_libraryModel);

	this->setItemDelegate(new LibraryItemDelegate(proxyModel));

	connect(_libraryModel, &LibraryModel::progressChanged, circleProgressBar, &QProgressBar::setValue);
	connect(_libraryModel, &LibraryModel::searchHasEnded, this, &LibraryTreeView::endPopulateTree);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(_libraryModel, &LibraryModel::trackCreated, this, &LibraryTreeView::insertTrack2);
	connect(_libraryModel, &LibraryModel::loadedFromFile, this, &LibraryTreeView::endPopulateTree);

	qDebug() << Q_FUNC_INFO;
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
			_libraryModel->invisibleRootItem()->appendRow(new LibraryItemLetter(letter));
		}
	}
}

void LibraryTreeView::insertTrack(const QString &absFilePath, const QString &artist, const QString &artistAlbum, const QString &album,
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
	case Settings::Artist:
		// Level 1
		if (_artists.contains(theArtist.toLower())) {
			itemArtist = _artists.value(theArtist.toLower());
			existingArtist = true;
		} else {
			itemArtist = new LibraryItemArtist(theArtist);
			_artists.insert(theArtist.toLower(), itemArtist);
			_libraryModel->invisibleRootItem()->appendRow(itemArtist);
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
	case Settings::Album:
		// Level 1
		if (_albums2.contains(album)) {
			itemAlbum = _albums2.value(album);
		} else {
			itemAlbum = new LibraryItemAlbum(album);
			_albums2.insert(album, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			this->insertLetter(album);
		}
		// Level 2
		itemTrack = new LibraryItemTrack(title);
		itemAlbum->appendRow(itemTrack);
		break;
	case Settings::ArtistAlbum:
		// Level 1
		if (_albums2.contains(theArtist + album)) {
			itemAlbum = _albums2.value(theArtist + album);
		} else {
			itemAlbum = new LibraryItemAlbum(theArtist + " – " + album);
			_albums2.insert(theArtist + album, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
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
	case Settings::Year:
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
			_libraryModel->invisibleRootItem()->appendRow(itemYear);
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
	//_persistentItems.insert(persistentItem);
}

void LibraryTreeView::insertTrack2(PersistentItem *item)
{
	/// TODO
	qDebug() << item->absoluteFilePath();
}

void LibraryTreeView::setModel(QAbstractItemModel *model)
{
	_libraryModel = qobject_cast<LibraryModel*>(model);
	TreeView::setModel(proxyModel);
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	LibraryItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
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
	LibraryItemDelegate *delegate = qobject_cast<LibraryItemDelegate *>(itemDelegate());
	if (delegate) {
		QModelIndex sourceIndex = proxyModel->mapToSource(index);
		LibraryItem *item = _libraryModel->itemFromIndex(sourceIndex);
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
	//circleProgressBar->show();
	/// XXX Behold with shared model between multiple views
	//_libraryModel->clear();
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
	/*if (musicLocationHasChanged) {
		emit searchMusic();
	} else if (QFile::exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp"))) {
		_libraryModel->loadFromFile();
	} else {
		// If the file has been erased from the disk meanwhile
		this->beginPopulateTree(true);
	}*/
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

/** Rebuild a subset of the tree. */
void LibraryTreeView::rebuild(QList<QPersistentModelIndex> indexes)
{
	// Parse once again those items
	foreach (QPersistentModelIndex index, indexes) {
		//LibraryItem *item = libraryModel->itemFromIndex(index);
		//if (item) {
			//int i = item->data(LibraryItem::IDX_TO_ABS_PATH).toInt();
			//QString file = item->data(LibraryItem::REL_PATH_TO_MEDIA).toString();
			//item->filePath();
			//libraryModel->readFile(file);
		//}
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
	qDebug() << Q_FUNC_INFO;
	sortByColumn(0, Qt::AscendingOrder);
	circleProgressBar->hide();
	circleProgressBar->setValue(0);
}
