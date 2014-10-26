#include "librarytreeview.h"
#include "settingsprivate.h"

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

#include "../circleprogressbar.h"
#include "../pluginmanager.h"
#include "styling/imageutils.h"
#include "jumptowidget.h"
#include "libraryitemdelegate.h"
#include "libraryorderdialog.h"
#include "libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryscrollbar.h"

#include <filehelper.h>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new QStandardItemModel(parent)), _db(NULL)
{
	_libraryModel->setColumnCount(1);

	int iconSize = SettingsPrivate::instance()->coverSize();
	this->setFrameShape(QFrame::NoFrame);
	this->setIconSize(QSize(iconSize, iconSize));

	_proxyModel = new LibraryFilterProxyModel(this);
	_proxyModel->setSourceModel(_libraryModel);
	_proxyModel->setTopLevelItems(&_topLevelItems);
	_itemDelegate = new LibraryItemDelegate(this, _proxyModel);
	this->setItemDelegate(_itemDelegate);

	_timer = new QTimer(this);
	_timer->setTimerType(Qt::PreciseTimer);
	_timer->setInterval(10);

	_circleProgressBar = new CircleProgressBar(this);
	_circleProgressBar->setTransparentCenter(true);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Send to the tag editor"), this);
	_properties = new QMenu(this);
	_properties->addAction(actionSendToCurrentPlaylist);
	_properties->addSeparator();
	_properties->addAction(actionOpenTagEditor);

	sendToCurrentPlaylist = new QShortcut(this);
	openTagEditor = new QShortcut(this);

	connect(this, &QTreeView::doubleClicked, [=] (const QModelIndex &) { appendToPlaylist(); });
	connect(_proxyModel, &LibraryFilterProxyModel::aboutToHighlight, this, [=](const QModelIndex &index, bool b) {
		if (!SettingsPrivate::instance()->isSearchAndExcludeLibrary()) {
			if (QStandardItem *item = _libraryModel->itemFromIndex(index)) {
				item->setData(b, DF_Highlighted);
			}
		}
	});

	// Context menu and shortcuts
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	connect(sendToCurrentPlaylist, &QShortcut::activated, this, &TreeView::appendToPlaylist);
	connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
	connect(openTagEditor, &QShortcut::activated, this, &TreeView::openTagEditor);

	_proxyModel->sortOrder();

	sortByColumn(0, Qt::AscendingOrder);
	setTextElideMode(Qt::ElideRight);

	_jumpToWidget = new JumpToWidget(this);
	_jumpToWidget->setBackgroundRole(QPalette::Button);
}

/** For every item in the library, gets the top level letter attached to it. */
QChar LibraryTreeView::currentLetter() const
{
	QModelIndex iTop = indexAt(viewport()->rect().topLeft());
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(iTop));

	// Special item "Various" (on top) has no Normalized String
	if (item && item->type() == IT_Letter && iTop.data(DF_NormalizedString).toString().isEmpty()) {
		return QChar();
	} else if (!iTop.isValid()) {
		return QChar();
	} else {
		// An item without a valid parent is a top level item, therefore we can extract the letter.
		while (iTop.parent().isValid()) {
			iTop = iTop.parent();
		}
		return iTop.data(DF_NormalizedString).toString().toUpper().at(0);
	}
}

/** Reimplemented. */
void LibraryTreeView::findAll(const QModelIndex &index, QStringList &tracks) const
{
	if (_itemDelegate) {
		QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
		if (item && item->hasChildren()) {
			for (int i = 0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), tracks);
			}
			tracks.removeDuplicates();
		} else if (item && item->type() == IT_Track) {
			tracks << item->data(DF_URI).toString();
		}
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

void LibraryTreeView::updateSelectedTracks()
{
	qDebug() << "LibraryTreeView: model has been updated, redraw selected tracks";
	//foreach (QModelIndex index, _cacheSelectedIndexes) {
	//	_itemDelegate->invalidate(index);
	//}
	/// Like the tagEditor, it's easier to proceed with complete clean/rebuild from dabatase
	_db->load();
}

void LibraryTreeView::init(SqlDatabase *db)
{
	_db = db;
	SettingsPrivate *settings = SettingsPrivate::instance();

	_proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(SettingsPrivate::FF_Menu), Qt::FontRole);
	this->setModel(_proxyModel);

	QObjectList objetsToExtend = QObjectList() << _properties << this;
	PluginManager::getInstance()->registerExtensionPoint(metaObject()->className(), objetsToExtend);

	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	vScrollBar->setFrameBorder(false, false, false, true);
	this->setVerticalScrollBar(vScrollBar);
	connect(vScrollBar, &LibraryScrollBar::aboutToDisplayItemDelegate, [=](bool b) {
		_itemDelegate->displayIcon(b);
		b ? _timer->start() : _timer->stop();
	});
	connect(_jumpToWidget, &JumpToWidget::displayItemDelegate, [=](bool b) {
		_itemDelegate->displayIcon(b);
		b ? _timer->start() : _timer->stop();
	});
	connect(_timer, &QTimer::timeout, this, &LibraryTreeView::repaintIcons);

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(_db, &SqlDatabase::aboutToLoad, this, &LibraryTreeView::reset);
	connect(_db, &SqlDatabase::loaded, this, &LibraryTreeView::endPopulateTree);
	connect(_db, &SqlDatabase::progressChanged, _circleProgressBar, &QProgressBar::setValue);
	connect(_db, &SqlDatabase::nodeExtracted, this, &LibraryTreeView::insertNode);
	connect(_db, &SqlDatabase::aboutToUpdateNode, this, &LibraryTreeView::updateNode);
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		foreach (QAction *action, _properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
			action->setFont(SettingsPrivate::instance()->font(SettingsPrivate::FF_Menu));
		}
		if (item->type() != IT_Letter) {
			_properties->exec(event->globalPos());
		}
	}
}

void LibraryTreeView::drawBranches(QPainter *painter, const QRect &r, const QModelIndex &proxyIndex) const
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->isBigCoverEnabled()) {
		QModelIndex index2 = proxyIndex;
		QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(proxyIndex));
		//if (item->data(Type).toInt() == Track) {
		//	item = item->parent();
		//	index2 = proxyIndex.parent();
		//}
		//QRect r2 = visualRect(index2);
		if (item && item->type() == IT_Album && isExpanded(index2)) {
			QString cover = item->data(DF_CoverPath).toString();
			// Get the area to display cover
			int w, h;
			w = rect().width() - (r.width() + 2 * verticalScrollBar()->width());
			h = item->rowCount() * this->indexRowSizeHint(index2.child(0, 0));
			QPixmap pixmap(cover);

			w = qMin(h, qMin(w, pixmap.width()));
			QPixmap leftBorder = pixmap.copy(0, 0, 3, pixmap.height());
			qDebug() << "leftBorder" << leftBorder.isNull();
			leftBorder = leftBorder.scaled(1 + rect().width() - (w + 2 * verticalScrollBar()->width()), w);
			// Create a mix with 2 images: first one is a 3 pixels subimage of the album cover which is expanded to the left border
			// The second one is a computer generated gradient focused on alpha channel
			if (!leftBorder.isNull()) {
				QLinearGradient linearAlphaBrush(0, 0, leftBorder.width(), 0);
				linearAlphaBrush.setColorAt(0, QApplication::palette().base().color());
				linearAlphaBrush.setColorAt(1, Qt::transparent);

				painter->save();
				// Because the expanded border can look strange to one, is blurred with some gaussian function
				QImage img = ImageUtils::blurred(leftBorder.toImage(), leftBorder.rect(), 10, false);
				painter->drawImage(0, r.y() + r.height(), img);
				painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
				painter->setPen(Qt::NoPen);
				painter->setBrush(linearAlphaBrush);
				painter->drawRect(0, r.y() + r.height(), leftBorder.width(), leftBorder.height());
				painter->drawPixmap(1 + rect().width() - (w + 2 * verticalScrollBar()->width()), r.y() + r.height(), w, w, pixmap);

				painter->setOpacity(settings->bigCoverOpacity());
				painter->fillRect(0, r.y() + r.height(), rect().width() - 2 * verticalScrollBar()->width(), leftBorder.height(), QApplication::palette().base());
				painter->restore();
			}
		}
	}
	TreeView::drawBranches(painter, r, proxyIndex);
}

/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
/*void LibraryTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
	// Save the position of the mouse, to be able to choose the correct action :
	// - add an item to the playlist
	// - edit stars to the current track
	currentPos = event->pos();
	QTreeView::mouseDoubleClickEvent(event);
}*/

void LibraryTreeView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		_jumpToWidget->move(frameGeometry().right() - 19 - wVerticalScrollBar, header()->height());
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, header()->height());
	}
	TreeView::paintEvent(event);
}

void LibraryTreeView::bindCoverToAlbum(QStandardItem *itemAlbum, const QString &album, const QString &absFilePath)
{
	QSqlQuery internalCover("SELECT DISTINCT album FROM tracks WHERE album = ? AND internalCover = 1", *_db);
	internalCover.addBindValue(album);
	if (!_db->isOpen()) {
		_db->open();
	}
	internalCover.exec();
	if (internalCover.next()) {
		itemAlbum->setData(absFilePath, DF_CoverPath);
	} else {
		QSqlQuery externalCover("SELECT DISTINCT cover FROM tracks WHERE album = ?", *_db);
		externalCover.addBindValue(album);
		externalCover.exec();
		if (externalCover.next()) {
			itemAlbum->setData(externalCover.record().value(0).toString(), DF_CoverPath);
		}
	}
}

/** Recursive count for leaves only. */
int LibraryTreeView::count(const QModelIndex &index) const
{
	if (_itemDelegate) {
		QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
		if (item) {
			int tmp = 0;
			for (int i = 0; i < item->rowCount(); i++) {
				tmp += count(index.child(i, 0));
			}
			return (tmp == 0) ? 1 : tmp;
		}
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

LetterItem* LibraryTreeView::insertLetter(const QString &letters)
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
			LetterItem *itemLetter = new LetterItem(letter);
			if (topLevelLetter) {
				itemLetter->setData("", DF_NormalizedString);
			} else {
				itemLetter->setData(letter, DF_NormalizedString);
			}
			_libraryModel->invisibleRootItem()->appendRow(itemLetter);
			_letters.insert(letter, itemLetter);
			return itemLetter;
		}
	}
	return NULL;
}

/** Invert the current sort order. */
void LibraryTreeView::changeSortOrder()
{
	if (_proxyModel->sortOrder() == Qt::AscendingOrder) {
		sortByColumn(0, Qt::DescendingOrder);
	} else {
		sortByColumn(0, Qt::AscendingOrder);
	}
}

/** Redraw the treeview with a new display mode. */
void LibraryTreeView::changeHierarchyOrder()
{
	qDebug() << Q_FUNC_INFO;
	// _db->load();
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (filter.isEmpty()) {
		_proxyModel->setFilterRegExp(QRegExp());
		_proxyModel->sort(0, _proxyModel->sortOrder());
	} else {
		bool needToSortAgain = false;
		if (_proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		_proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
			_proxyModel->sort(0, _proxyModel->sortOrder());
		}
	}
}

/** Find index from current letter then scrolls to it. */
void LibraryTreeView::jumpTo(const QString &letter)
{
	QStandardItem *item = _letters.value(letter);
	if (item) {
		scrollTo(_proxyModel->mapFromSource(item->index()), PositionAtTop);
	}
}

/** Reimplemented. */
void LibraryTreeView::reset()
{
	_circleProgressBar->show();
	if (_libraryModel->rowCount() > 0) {
		_proxyModel->setFilterRegExp(QString());
		_letters.clear();
		_libraryModel->removeRows(0, _libraryModel->rowCount());
		_topLevelItems.clear();
		QMapIterator<GenericDAO*, QStandardItem*> it(_map);
		while (it.hasNext()) {
			it.next();
			delete it.key();
		}
	}
	switch (SettingsPrivate::instance()->value("insertPolicy").toInt()) {
	case SqlDatabase::IP_Artists:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists \\ Albums"));
		break;
	case SqlDatabase::IP_Albums:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Albums"));
		break;
	case SqlDatabase::IP_ArtistsAlbums:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists – Albums"));
		break;
	case SqlDatabase::IP_Years:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Years"));
		break;
	}
}

void LibraryTreeView::endPopulateTree()
{
	sortByColumn(0, Qt::AscendingOrder);
	_circleProgressBar->hide();
	_circleProgressBar->setValue(0);
	QMapIterator<GenericDAO*, QStandardItem*> it(_map);
	while (it.hasNext()) {
		it.next();
		delete it.key();
	}
	_map.clear();
}

/** Find and insert a node in the hierarchy of items. */
void LibraryTreeView::insertNode(GenericDAO *node)
{
	QStandardItem *nodeItem = NULL;
	if (TrackDAO *dao = qobject_cast<TrackDAO*>(node)) {
		nodeItem = new TrackItem(dao);
	} else if (AlbumDAO *dao = qobject_cast<AlbumDAO*>(node)) {
		nodeItem = new AlbumItem(dao);
	} else if (ArtistDAO *dao = qobject_cast<ArtistDAO*>(node)) {
		nodeItem = new ArtistItem(dao);
	}

	if (node->parentNode()) {
		QStandardItem *parentItem = _map.value(node->parentNode());
		parentItem->appendRow(nodeItem);
	} else {
		_libraryModel->invisibleRootItem()->appendRow(nodeItem);
		LetterItem *letter = this->insertLetter(nodeItem->text());
		_topLevelItems.insert(letter->index(), nodeItem->index());
	}
	_map.insert(node, nodeItem);
}


void LibraryTreeView::updateNode(GenericDAO *)
{
	qDebug() << Q_FUNC_INFO << "not implemented";
}
