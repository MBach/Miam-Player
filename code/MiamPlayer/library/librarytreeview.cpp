#include "librarytreeview.h"
#include "settings.h"

#include <QApplication>
#include <QDirIterator>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThread>

#include <QtDebug>

#include "../library/libraryitemdelegate.h"

#include <filehelper.h>

#include "libraryscrollbar.h"

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new QStandardItemModel(parent)), sqlModel(NULL)
{
	_libraryModel->setColumnCount(1);

	int iconSize = Settings::getInstance()->coverSize();
	this->setFrameShape(QFrame::NoFrame);
	this->setIconSize(QSize(iconSize, iconSize));

	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSourceModel(_libraryModel);
	proxyModel->setTopLevelItems(&_topLevelItems);
	_itemDelegate = new LibraryItemDelegate(proxyModel);
	this->setItemDelegate(_itemDelegate);

	_timer = new QTimer(this);
	_timer->setTimerType(Qt::PreciseTimer);
	_timer->setInterval(10);

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

	proxyModel->sortOrder();

	sortByColumn(0, Qt::AscendingOrder);
	setTextElideMode(Qt::ElideRight);

	_jumpToWidget = new JumpToWidget(this);
	_jumpToWidget->setBackgroundRole(QPalette::Button);
}

/** For every item in the library, gets the top level letter attached to it. */
QChar LibraryTreeView::currentLetter() const
{
	QModelIndex iTop = indexAt(viewport()->rect().topLeft());
	if (iTop.data(Type).toInt() == Letter && iTop.row() == 0 && proxyModel->sortOrder() == Qt::AscendingOrder) {
		return QChar();
	} else {
		// An item without a valid parent is a top level item, therefore we can extract the letter.
		while (iTop.parent().isValid()) {
			iTop = iTop.parent();
		}
		return iTop.data(DataNormalizedString).toString().toUpper().at(0);
	}
}

void LibraryTreeView::repaintIcons()
{
	static qreal r = 0;
	if (_timer->isActive()) {
		r += 0.01;
		_itemDelegate->setIconOpacity(r);
		if (r >= 1) {
			_timer->stop();
			r = 0;
		}
		this->viewport()->repaint();
	}
}

void LibraryTreeView::init(LibrarySqlModel *sql)
{
	sqlModel = sql;
	Settings *settings = Settings::getInstance();

	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(Settings::MENUS), Qt::FontRole);
	this->setModel(proxyModel);
	qDebug() << Q_FUNC_INFO << viewport()->geometry();

	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	this->setVerticalScrollBar(vScrollBar);
	connect(vScrollBar, &LibraryScrollBar::displayItemDelegate, [=](bool b) {
		_itemDelegate->displayIcon(b);
		b ? _timer->start() : _timer->stop();
	});
	connect(_jumpToWidget, &JumpToWidget::displayItemDelegate, [=](bool b) {
		_itemDelegate->displayIcon(b);
		b ? _timer->start() : _timer->stop();
	});
	connect(_timer, &QTimer::timeout, this, &LibraryTreeView::repaintIcons);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(sqlModel, &LibrarySqlModel::coverWasUpdated, this, &LibraryTreeView::updateCover);
	connect(sqlModel, &LibrarySqlModel::modelAboutToBeReset, this, &LibraryTreeView::reset);
	connect(sqlModel, &LibrarySqlModel::modelReset, this, &LibraryTreeView::endPopulateTree);
	connect(sqlModel, &LibrarySqlModel::progressChanged, circleProgressBar, &QProgressBar::setValue);
	connect(sqlModel, &LibrarySqlModel::trackExtractedFromDB, this, &LibraryTreeView::insertTrackFromRecord);
	connect(sqlModel, &LibrarySqlModel::trackExtractedFromFS, this, &LibraryTreeView::insertTrackFromFile);

	// One can choose a hierarchical order for drawing the library
	/*LibraryOrderDialog *_lod = new LibraryOrderDialog(this);
	connect(header(), &QHeaderView::customContextMenuRequested, [=](const QPoint &pos) {
		_lod->move(mapToGlobal(pos));
		_lod->show();
	});
	connect(_lod, &LibraryOrderDialog::aboutToRedrawLibrary, sqlModel, &LibrarySqlModel::load);*/
}

void LibraryTreeView::insertTrackFromFile(const FileHelper &fh)
{
	this->insertTrack(fh.fileInfo().absoluteFilePath(), fh.artistAlbum(), fh.artist(), fh.album(), fh.discNumber(),
					  fh.title(), fh.trackNumber().toInt(), fh.year().toInt());
}

void LibraryTreeView::insertTrackFromRecord(const QSqlRecord &record)
{
	int i = -1;
	const QString artist = record.value(++i).toString();
	const QString artistAlbum = record.value(++i).toString();
	const QString album = record.value(++i).toString();
	const QString title = record.value(++i).toString();
	int trackNumber = record.value(++i).toInt();
	int discNumber = record.value(++i).toInt();
	int year = record.value(++i).toInt();
	const QString absFilePath = record.value(++i).toString();
	this->insertTrack(absFilePath, artistAlbum, artist, album, discNumber, title, trackNumber, year);
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		foreach (QAction *action, properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
		}
		if (item->data(Type).toInt() != Letter) {
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

void LibraryTreeView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		_jumpToWidget->move(frameGeometry().right() - 19 - wVerticalScrollBar, 1 + header()->height());
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, 1 + header()->height());
	}
	TreeView::paintEvent(event);
}

void LibraryTreeView::bindCoverToAlbum(QStandardItem *itemAlbum, const QString &album, const QString &absFilePath)
{
	QSqlQuery internalCover("SELECT DISTINCT album FROM tracks WHERE album = ? AND internalCover = 1", sqlModel->database());
	internalCover.addBindValue(album);
	if (!sqlModel->database().isOpen()) {
		sqlModel->database().open();
	}
	internalCover.exec();
	if (internalCover.next()) {
		itemAlbum->setData(absFilePath, DataCoverPath);
	} else {
		QSqlQuery externalCover("SELECT DISTINCT coverAbsPath FROM tracks WHERE album = ?", sqlModel->database());
		externalCover.addBindValue(album);
		externalCover.exec();
		if (externalCover.next()) {
			itemAlbum->setData(externalCover.record().value(0).toString(), DataCoverPath);
		}
	}
}

/** Recursive count for leaves only. */
int LibraryTreeView::count(const QModelIndex &index) const
{
	if (_itemDelegate) {
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
	if (_itemDelegate) {
		QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(index));
		if (item->hasChildren()) {
			for (int i=0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), tracks);
			}
		} else if (item->data(Type).toInt() == Track) {
			tracks.append(item->data(DataAbsFilePath).toString());
		}
	}
}

QStandardItem* LibraryTreeView::insertLetter(const QString &letters)
{
	if (!letters.isEmpty()) {
		QString c = letters.left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		QString letter;
		bool topLevelLetter = false;
		if (c.contains(QRegExp("\\w"))) {
			letter = c;
		} else {
			letter = tr("Various");
			topLevelLetter = true;
		}
		if (_letters.contains(letter)) {
			return _letters.value(letter);
		} else {
			QStandardItem *itemLetter = new QStandardItem(letter);
			itemLetter->setData(Letter, Type);
			if (topLevelLetter) {
				itemLetter->setData("", DataNormalizedString);
			} else {
				itemLetter->setData(letter, DataNormalizedString);
			}
			_libraryModel->invisibleRootItem()->appendRow(itemLetter);
			_letters.insert(letter, itemLetter);
			return itemLetter;
		}
	}
	return NULL;
}

void LibraryTreeView::insertTrack(const QString &absFilePath, const QString &artistAlbum, const QString &artist,
								  const QString &album, int discNumber, const QString &title, int trackNumber, int year)
{
	QString theArtist = artistAlbum.isEmpty() ? artist : artistAlbum;

	QStandardItem *itemArtist = NULL;
	QStandardItem *itemAlbum = NULL;
	QStandardItem *itemTrack = NULL;
	QStandardItem *itemYear = NULL;

	QString art = artist.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
	QString theArtistNorm = theArtist.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
	QString alb = album.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();

	static bool existingArtist = true;
	switch (Settings::getInstance()->value("insertPolicy").toInt()) {
	case Artist:
		// Level 1
		if (_artists.contains(theArtist.toLower())) {
			itemArtist = _artists.value(theArtist.toLower());
			existingArtist = true;
		} else {
			itemArtist = new QStandardItem(theArtist);
			itemArtist->setData(Artist, Type);
			itemArtist->setData(art, DataNormalizedString);
			_artists.insert(theArtist.toLower(), itemArtist);
			_libraryModel->invisibleRootItem()->appendRow(itemArtist);
			QStandardItem *letter = this->insertLetter(art);
			if (letter) {
				_topLevelItems.insert(letter->index(), itemArtist->index());
			}
			existingArtist = false;
		}
		// Level 2
		if (existingArtist && _albums.contains(QPair<QStandardItem*, QString>(itemArtist, alb))) {
			itemAlbum = _albums.value(QPair<QStandardItem*, QString>(itemArtist, alb));
		} else {
			itemAlbum = new QStandardItem(album);
			itemAlbum->setData(Album, Type);
			itemAlbum->setData(alb, DataNormalizedString);
			itemAlbum->setData(year, DataYear);
			this->bindCoverToAlbum(itemAlbum, album, absFilePath);
			_albums.insert(QPair<QStandardItem *, QString>(itemArtist, alb), itemAlbum);
			_albums2.insert(alb, itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3 (option)
		if (discNumber > 0 && !_discNumbers.contains(QPair<QStandardItem*, int>(itemAlbum, discNumber))) {
			QStandardItem *itemDiscNumber = new QStandardItem(QString::number(discNumber));
			itemDiscNumber->setData(Disc, Type);
			itemDiscNumber->setData(discNumber, DataDiscNumber);
			_discNumbers.insert(QPair<QStandardItem *, int>(itemAlbum, discNumber), itemDiscNumber);
			itemAlbum->appendRow(itemDiscNumber);
		}
		// Level 3
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new QStandardItem(title);
		} else {
			itemTrack = new QStandardItem(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Album:
		// Level 1
		if (_albums2.contains(alb)) {
			itemAlbum = _albums2.value(alb);
		} else {
			itemAlbum = new QStandardItem(album);
			itemAlbum->setData(Album, Type);
			itemAlbum->setData(alb, DataNormalizedString);
			itemAlbum->setData(year, DataYear);
			this->bindCoverToAlbum(itemAlbum, album, absFilePath);
			_albums2.insert(alb, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			QStandardItem *letter = this->insertLetter(alb);
			if (letter) {
				_topLevelItems.insert(letter->index(), itemAlbum->index());
			}
		}
		// Level 2
		itemTrack = new QStandardItem(title);
		itemAlbum->appendRow(itemTrack);
		break;
	case ArtistAlbum:
		// Level 1
		if (_albums2.contains(theArtistNorm + alb)) {
			itemAlbum = _albums2.value(theArtistNorm + alb);
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + album);
			itemAlbum->setData(Album, Type);
			itemAlbum->setData(theArtistNorm + alb, DataNormalizedString);
			itemAlbum->setData(year, DataYear);
			this->bindCoverToAlbum(itemAlbum, album, absFilePath);
			_albums2.insert(theArtistNorm + alb, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			QStandardItem *letter = this->insertLetter(theArtistNorm + alb);
			if (letter) {
				_topLevelItems.insert(letter->index(), itemAlbum->index());
			}
		}
		// Level 2
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new QStandardItem(title);
		} else {
			itemTrack = new QStandardItem(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case Year:
		// Level 1
		if (_years.contains(year)) {
			itemYear = _years.value(year);
		} else {
			if (year > 0) {
				itemYear = new QStandardItem(QString::number(year));
			} else {
				itemYear = new QStandardItem(tr("Unknown"));
			}
			itemYear->setData(Year, Type);
			itemYear->setData(year, DataNormalizedString);
			_years.insert(year, itemYear);
			_libraryModel->invisibleRootItem()->appendRow(itemYear);
		}
		// Level 2
		if (_albums2.contains(theArtist + alb)) {
			itemAlbum = _albums2.value(theArtist + alb);
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + album);
			itemAlbum->setData(Album, Type);
			itemAlbum->setData(art.append("|").append(alb), DataNormalizedString);
			itemAlbum->setData(year, DataYear);
			this->bindCoverToAlbum(itemAlbum, album, absFilePath);
			_albums2.insert(theArtist + alb, itemAlbum);
			itemYear->appendRow(itemAlbum);
		}
		// Level 3
		if (artistAlbum.isEmpty() || QString::compare(artist, artistAlbum) == 0) {
			itemTrack = new QStandardItem(title);
		} else {
			itemTrack = new QStandardItem(title + " (" + artist + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	}
	/// XXX: Is it necessary to create subclasses of QStandardItem for item->type()?
	// itemTrack always exists
	itemTrack->setData(Track, Type);
	itemTrack->setData(absFilePath, DataAbsFilePath);
	itemTrack->setData(trackNumber, DataTrackNumber);
}

void LibraryTreeView::updateCover(const QFileInfo &coverFileInfo)
{
	QSqlQuery externalCover("SELECT DISTINCT album FROM tracks WHERE path = ?", sqlModel->database());
	externalCover.addBindValue(QDir::toNativeSeparators(coverFileInfo.absolutePath()));
	if (!sqlModel->database().isOpen()) {
		sqlModel->database().open();
	}
	externalCover.exec();
	if (externalCover.next()) {
		QString album = externalCover.record().value(0).toString();
		QString alb = album.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
		QStandardItem *itemAlbum = _albums2.value(alb);
		if (itemAlbum) {
			itemAlbum->setData(coverFileInfo.absoluteFilePath(), DataCoverPath);
		}
	}
}

/** Invert the current sort order. */
void LibraryTreeView::changeSortOrder()
{
	if (proxyModel->sortOrder() == Qt::AscendingOrder) {
		sortByColumn(0, Qt::DescendingOrder);
	} else {
		sortByColumn(0, Qt::AscendingOrder);
	}
}

/** Redraw the treeview with a new display mode. */
void LibraryTreeView::changeHierarchyOrder()
{
	sqlModel->load();
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (filter.isEmpty()) {
		proxyModel->setFilterRegExp(QRegExp());
		collapseAll();
		proxyModel->sort(0, proxyModel->sortOrder());
	} else {
		bool needToSortAgain = false;
		if (proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
			collapseAll();
			proxyModel->sort(0, proxyModel->sortOrder());
		}
	}
}

/** Find index from current letter then scrolls to it. */
void LibraryTreeView::jumpTo(const QString &letter)
{
	QStandardItem *item = _letters.value(letter);
	if (item) {
		scrollTo(proxyModel->mapFromSource(item->index()), PositionAtTop);
	}
}

/** Reimplemented. */
void LibraryTreeView::reset()
{
	circleProgressBar->show();
	if (_libraryModel->rowCount() > 0) {		
		_artists.clear();
		_albums.clear();
		_discNumbers.clear();
		_albums2.clear();
		_albumsAbsPath.clear();
		_artistsAlbums.clear();
		_years.clear();
		_letters.clear();
		_libraryModel->removeRows(0, _libraryModel->rowCount());
		_topLevelItems.clear();
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

void LibraryTreeView::endPopulateTree()
{
	sortByColumn(0, Qt::AscendingOrder);
	circleProgressBar->hide();
	circleProgressBar->setValue(0);
}
