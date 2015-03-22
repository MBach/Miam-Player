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
#include "library/jumptowidget.h"
#include "library/libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryorderdialog.h"
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
				item->setData(b, Miam::DF_Highlighted);
			}
		}
	});

	// Context menu and shortcuts
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	connect(sendToCurrentPlaylist, &QShortcut::activated, this, &TreeView::appendToPlaylist);
	connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
	connect(openTagEditor, &QShortcut::activated, this, &TreeView::openTagEditor);

	sortByColumn(0, Qt::AscendingOrder);
	setTextElideMode(Qt::ElideRight);

	_jumpToWidget = new JumpToWidget(this);
	_jumpToWidget->setBackgroundRole(QPalette::Button);
	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, &LibraryTreeView::jumpTo);
	//connect(this, &LibraryTreeView::currentLetterChanged, _jumpToWidget, &JumpToWidget::setCurrentLetter);
}

/** For every item in the library, gets the top level letter attached to it. */
QChar LibraryTreeView::currentLetter() const
{
	QModelIndex iTop = indexAt(viewport()->rect().topLeft());
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(iTop));

	// Special item "Various" (on top) has no Normalized String
	if (item && item->type() == Miam::IT_Separator && iTop.data(Miam::DF_NormalizedString).toString() == "0") {
		return QChar();
	} else if (!iTop.isValid()) {
		return QChar();
	} else {
		// An item without a valid parent is a top level item, therefore we can extract the letter.
		while (iTop.parent().isValid()) {
			iTop = iTop.parent();
		}
		if (iTop.isValid() && !iTop.data(Miam::DF_NormalizedString).toString().isEmpty()) {
			return iTop.data(Miam::DF_NormalizedString).toString().toUpper().at(0);
		} else {
			return QChar();
		}
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
		} else if (item && item->type() == Miam::IT_Track) {
			tracks << item->data(Miam::DF_URI).toString();
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
	PluginManager::instance()->registerExtensionPoint(metaObject()->className(), objetsToExtend);

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
}

void LibraryTreeView::setVisible(bool visible)
{
	TreeView::setVisible(visible);
	disconnect(_db, 0, this, 0);
	disconnect(_db, 0, _circleProgressBar, 0);
	if (visible) {
		connect(_db, &SqlDatabase::aboutToLoad, this, &LibraryTreeView::reset);
		connect(_db, &SqlDatabase::loaded, this, &LibraryTreeView::endPopulateTree);
		connect(_db, &SqlDatabase::progressChanged, _circleProgressBar, &QProgressBar::setValue);
		connect(_db, &SqlDatabase::nodeExtracted, this, &LibraryTreeView::insertNode);
		connect(_db, &SqlDatabase::aboutToUpdateNode, this, &LibraryTreeView::updateNode);
	}
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
		if (item->type() != Miam::IT_Separator) {
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
		if (item && item->type() == Miam::IT_Album && isExpanded(index2)) {
			QString cover = item->data(Miam::DF_CoverPath).toString();
			// Get the area to display cover
			int w, h;
			w = rect().width() - (r.width() + 2 * verticalScrollBar()->width());
			h = item->rowCount() * this->indexRowSizeHint(index2.child(0, 0));
			QPixmap pixmap(cover);

			w = qMin(h, qMin(w, pixmap.width()));
			QPixmap leftBorder = pixmap.copy(0, 0, 3, pixmap.height());
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
	///XXX: analyze performance?
	_jumpToWidget->setCurrentLetter(currentLetter());
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

SeparatorItem *LibraryTreeView::insertSeparator(const QString &letters)
{
	// Items are grouped every ten years in this particular case
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Years: {
		int year = letters.toInt();
		if (year == 0) {
			return NULL;
		}
		QString yearStr = QString::number(year - year % 10);
		if (_letters.contains(yearStr)) {
			return _letters.value(yearStr);
		} else {
			SeparatorItem *separator = new SeparatorItem(yearStr);
			separator->setData(yearStr, Miam::DF_NormalizedString);
			_libraryModel->invisibleRootItem()->appendRow(separator);
			_letters.insert(yearStr, separator);
			return separator;
		}
		break;
	}
	default:
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
			SeparatorItem *separator = new SeparatorItem(letter);
			if (topLevelLetter) {
				separator->setData("0", Miam::DF_NormalizedString);
			} else {
				separator->setData(letter.toLower(), Miam::DF_NormalizedString);
			}
			_libraryModel->invisibleRootItem()->appendRow(separator);
			_letters.insert(letter, separator);
			return separator;
		}
	}
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
	_db->load();
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
	if (sender() == NULL) {
		return;
	}
	_circleProgressBar->show();
	if (_libraryModel->rowCount() > 0) {
		_proxyModel->setFilterRegExp(QString());
		_letters.clear();
		_libraryModel->removeRows(0, _libraryModel->rowCount());
		_topLevelItems.clear();
		this->verticalScrollBar()->setValue(0);
		qDeleteAll(_map.begin(), _map.end());
		_map.clear();
	}
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Artists:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists \\ Albums"));
		break;
	case SettingsPrivate::IP_Albums:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Albums"));
		break;
	case SettingsPrivate::IP_ArtistsAlbums:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists – Albums"));
		break;
	case SettingsPrivate::IP_Years:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Years"));
		break;
	}
}

void LibraryTreeView::endPopulateTree()
{
	_proxyModel->sort(0);
	_proxyModel->setDynamicSortFilter(true);
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
	} else if (YearDAO *dao = qobject_cast<YearDAO*>(node)) {
		nodeItem = new YearItem(dao);
	}

	if (node->parentNode()) {
		QStandardItem *parentItem = _map.value(node->parentNode());
		if (parentItem) {
			parentItem->appendRow(nodeItem);
		} else {
			qDebug() << Q_FUNC_INFO << "parentItem should exists but it was not found?" << node->title();
		}
	} else {
		_libraryModel->invisibleRootItem()->appendRow(nodeItem);
		SeparatorItem *separator = this->insertSeparator(nodeItem->text());
		if (separator) {
			_topLevelItems.insert(separator->index(), nodeItem->index());
		}
	}
	_map.insert(node, nodeItem);
}

void LibraryTreeView::updateNode(GenericDAO *node)
{
	// Is it possible to update other types of nodes?
	if (AlbumItem *album = static_cast<AlbumItem*>(_map.value(node))) {
		AlbumDAO *dao = qobject_cast<AlbumDAO*>(node);
		album->setData(dao->year(), Miam::DF_Year);
		album->setData(dao->cover(), Miam::DF_CoverPath);
		album->setData(dao->icon(), Miam::DF_IconPath);
		album->setData(!dao->icon().isEmpty(), Miam::DF_IsRemote);
	}
}
