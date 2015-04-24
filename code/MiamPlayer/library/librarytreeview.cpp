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
#include "library/jumptowidget.h"
#include "library/libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryorderdialog.h"
#include "libraryitemdelegate.h"
#include "libraryscrollbar.h"

#include <filehelper.h>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new QStandardItemModel(parent))
{
	_libraryModel->setColumnCount(1);

	auto settings = SettingsPrivate::instance();
	int iconSize = settings->coverSize();
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
		if (!settings->isSearchAndExcludeLibrary()) {
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

	/*connect(this, &QTreeView::expanded, this, [=](const QModelIndex &index) {
		QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
		if (item->type() == Miam::IT_Album && settings->isBigCoverEnabled()) {
			this->setExpandedCover(item);
			// load album cover here
			//int h = item->rowCount() * rowHeight(item->child(0, 0)->index());
			//qDebug() << Q_FUNC_INFO << h << rect().width();
			//item->setData(QImage(item->data(Miam::DF_CoverPath).toString()).scaledToHeight(h), Miam::DF_Custom + 1);
			item->setData(QImage(item->data(Miam::DF_CoverPath).toString()), Miam::DF_Custom + 1);
		}
	});*/
	connect(this, &QTreeView::expanded, this, &LibraryTreeView::setExpandedCover);

	/*connect(this, &QTreeView::collapsed, this, [=](const QModelIndex &index) {
		QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
		if (item->type() == Miam::IT_Album) {
			// destroy album cover here
			//item->setData(QImage(), Miam::DF_Custom + 1);
			this->removeExpandedCover(item);
		}
	});*/
	connect(this, &QTreeView::collapsed, this, &LibraryTreeView::removeExpandedCover);
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

const QImage * LibraryTreeView::expandedCover(QStandardItem *album) const
{
	// proxy, etc
	return _expandedCovers.value(album);
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

void LibraryTreeView::removeExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		/// TODO: inner cover
		QImage *image = _expandedCovers.value(item);
		delete image;
		_expandedCovers.remove(item);
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

void LibraryTreeView::setExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		/// TODO: inner cover
		QImage *image = new QImage(item->data(Miam::DF_CoverPath).toString());
		_expandedCovers.insert(item, image);
	}
}

void LibraryTreeView::updateSelectedTracks()
{
	//foreach (QModelIndex index, _cacheSelectedIndexes) {
	//	_itemDelegate->invalidate(index);
	//}
	/// Like the tagEditor, it's easier to proceed with complete clean/rebuild from dabatase
	qDebug() << Q_FUNC_INFO;
	SqlDatabase::instance()->load();
}

void LibraryTreeView::init()
{
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

#include <functional>

/** Rebuild the list of separators when one has changed grammatical articles in options. */
void LibraryTreeView::rebuildSeparators()
{
	qDebug() << Q_FUNC_INFO;
	QHashIterator<SeparatorItem*, QModelIndex> i(_topLevelItems);
	while (i.hasNext()) {
		i.next();
		if (!i.value().data(Miam::DF_Custom).toString().isEmpty()) {
			auto item = _libraryModel->itemFromIndex(i.value());
			if (item) {
				qDebug() << item->text();
				item->setData(QString(), Miam::DF_Custom);
			}
		}
	}

	// Delete separators first
	QSet<int> setRows;
	QHashIterator<QString, SeparatorItem*> it(_letters);
	while (it.hasNext()) {
		it.next();
		setRows << it.value()->index().row();
	}

	// Always remove items (rows) in reverse order
	QList<int> rows = setRows.toList();
	std::sort(rows.begin(), rows.end(), std::greater<int>());
	for (int row : rows) {
		auto item = _libraryModel->takeItem(row);
		_libraryModel->removeRow(row);
		delete item;
	}
	_letters.clear();
	_topLevelItems.clear();

	// Insert once again new separators
	for (int row = 0; row < _libraryModel->rowCount(); row++) {
		auto item = _libraryModel->item(row);
		if (auto separator = this->insertSeparator(item)) {
			_topLevelItems.insert(separator, item->index());
		}
	}
}

void LibraryTreeView::setVisible(bool visible)
{
	TreeView::setVisible(visible);
	auto db = SqlDatabase::instance();
	disconnect(db, 0, this, 0);
	disconnect(db, 0, _circleProgressBar, 0);
	if (visible) {
		connect(db, &SqlDatabase::aboutToLoad, this, &LibraryTreeView::reset);
		connect(db, &SqlDatabase::loaded, this, &LibraryTreeView::endPopulateTree);
		connect(db, &SqlDatabase::progressChanged, _circleProgressBar, &QProgressBar::setValue);
		connect(db, &SqlDatabase::nodeExtracted, this, &LibraryTreeView::insertNode);
		connect(db, &SqlDatabase::aboutToUpdateNode, this, &LibraryTreeView::updateNode);
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

SeparatorItem *LibraryTreeView::insertSeparator(const QStandardItem *node)
{
	// Items are grouped every ten years in this particular case
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Years: {
		int year = node->text().toInt();
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
		QString c;
		if (node->data(Miam::DF_Custom).toString().isEmpty()) {
			c = node->text().left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		} else {
			QString reorderedText = node->data(Miam::DF_Custom).toString();
			c = reorderedText.left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		}
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
	qDebug() << Q_FUNC_INFO << sender();
	SqlDatabase::instance()->load();
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
		if (QStandardItem *parentItem = _map.value(node->parentNode())) {
			parentItem->appendRow(nodeItem);
		}
	} else if (!_map.contains(node)) {
		_libraryModel->invisibleRootItem()->appendRow(nodeItem);
		if (SeparatorItem *separator = this->insertSeparator(nodeItem)) {
			_topLevelItems.insert(separator, nodeItem->index());
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
