#include "librarytreeview.h"

#include <library/jumptowidget.h>
#include <cover.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include "../circleprogressbar.h"
#include "../pluginmanager.h"
#include "libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryorderdialog.h"
#include "libraryitemdelegate.h"
#include "libraryscrollbar.h"
#include "libraryfilterlineedit.h"

#include <functional>

#include <QtDebug>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new LibraryItemModel(parent)), _searchBar(NULL)
{
	auto settings = SettingsPrivate::instance();
	int iconSize = settings->coverSize();
	this->setFrameShape(QFrame::NoFrame);
	this->setIconSize(QSize(iconSize, iconSize));

	_proxyModel = new LibraryFilterProxyModel(this);
	_proxyModel->setSourceModel(_libraryModel);
	_proxyModel->setTopLevelItems(&_libraryModel->topLevelItems());
	_itemDelegate = new LibraryItemDelegate(this, _proxyModel);
	this->setItemDelegate(_itemDelegate);

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

	// Load album cover
	connect(this, &QTreeView::expanded, this, &LibraryTreeView::setExpandedCover);
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

void LibraryTreeView::findMusic(const QString &text)
{
	if (SettingsPrivate::instance()->librarySearchMode() == SettingsPrivate::LSM_Filter) {
		this->filterLibrary(text);
	} else {
		this->highlightMatchingText(text);
	}
}

void LibraryTreeView::removeExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		QImage *image = _expandedCovers.value(item);
		delete image;
		_expandedCovers.remove(item);
	}
}

void LibraryTreeView::setExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		QString coverPath = item->data(Miam::DF_CoverPath).toString();
		if (coverPath.isEmpty()) {
			return;
		}
		QImage *image;
		if (coverPath.startsWith("file://")) {
			FileHelper fh(coverPath);
			Cover *cover = fh.extractCover();
			if (cover) {
				image = new QImage();
				image->loadFromData(cover->byteArray(), cover->format());
				delete cover;
			}
		} else {
			image = new QImage(coverPath);
		}
		_expandedCovers.insert(item, image);
	}
}

void LibraryTreeView::updateSelectedTracks()
{
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

	connect(vScrollBar, &LibraryScrollBar::aboutToDisplayItemDelegate, _itemDelegate, &LibraryItemDelegate::displayIcon);
	connect(_jumpToWidget, &JumpToWidget::displayItemDelegate, _itemDelegate, &LibraryItemDelegate::displayIcon);
}

void LibraryTreeView::setSearchBar(LibraryFilterLineEdit *lfle) {
	_searchBar = lfle;
	// Reset filter or remove highlight when one is switching from one mode to another
	connect(SettingsPrivate::instance(), &SettingsPrivate::librarySearchModeChanged, this, [=](SettingsPrivate::LibrarySearchMode lsm) {
		QString text;
		_searchBar->setText(text);
		if (lsm == SettingsPrivate::LSM_Filter) {
			this->highlightMatchingText(text);
		} else {
			this->filterLibrary(text);
		}
	});
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
		connect(db, &SqlDatabase::nodeExtracted, _libraryModel, &LibraryItemModel::insertNode);
		connect(db, &SqlDatabase::aboutToUpdateNode, _libraryModel, &LibraryItemModel::updateNode);
		connect(db, &SqlDatabase::aboutToCleanView, _libraryModel, &LibraryItemModel::cleanDanglingNodes);
		//connect(db, &SqlDatabase::aboutToUpdateView, _libraryModel, &LibraryItemModel::updateNodes);
	}
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		for (QAction *action : _properties->actions()) {
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
		_jumpToWidget->move(frameGeometry().right() - 22 - wVerticalScrollBar, header()->height());
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
	for (QModelIndex index : indexes) {
		c += this->count(index);
	}
	return c;
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (filter.isEmpty()) {
		_proxyModel->setFilterRole(Qt::DisplayRole);
		_proxyModel->setFilterRegExp(QRegExp());
		_proxyModel->sort(0, _proxyModel->sortOrder());
	} else {
		bool needToSortAgain = false;
		if (_proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		if (filter.contains(QRegExp("^(\\*){1,5}$"))) {
			// Convert stars into [1-5], ..., [5-5] regular expression
			_proxyModel->setFilterRole(Miam::DF_Rating);
			_proxyModel->setFilterRegExp(QRegExp("[" + QString::number(filter.size()) + "-5]", Qt::CaseInsensitive, QRegExp::RegExp));
		} else {
			_proxyModel->setFilterRole(Qt::DisplayRole);
			_proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		}
		if (needToSortAgain) {
			_proxyModel->sort(0, _proxyModel->sortOrder());
		}
	}
}

/** Highlight items in the Tree when one has activated this option in settings. */
void LibraryTreeView::highlightMatchingText(const QString &text)
{
	// Clear highlight on every call
	std::function<void(QStandardItem *item)> recursiveClearHighlight;
	recursiveClearHighlight = [&recursiveClearHighlight] (QStandardItem *item) -> void {
		if (item->hasChildren()) {
			item->setData(false, Miam::DF_Highlighted);
			for (int i = 0; i < item->rowCount(); i++) {
				recursiveClearHighlight(item->child(i, 0));
			}
		} else {
			item->setData(false, Miam::DF_Highlighted);
		}
	};

	for (int i = 0; i < _libraryModel->rowCount(); i++) {
		recursiveClearHighlight(_libraryModel->item(i, 0));
	}

	// Adapt filter if one is typing '*'
	QString filter;
	Qt::MatchFlags flags;
	if (text.contains(QRegExp("^(\\*){1,5}$"))) {
		_proxyModel->setFilterRole(Miam::DF_Rating);
		filter = "[" + QString::number(text.size()) + "-5]";
		flags = Qt::MatchRecursive | Qt::MatchRegExp;
	} else {
		_proxyModel->setFilterRole(Qt::DisplayRole);
		filter = text;
		flags = Qt::MatchRecursive | Qt::MatchContains;
	}

	// Mark items with a bold font
	QSet<QChar> lettersToHighlight;
	if (!text.isEmpty()) {
		QModelIndexList indexes = _libraryModel->match(_libraryModel->index(0, 0, QModelIndex()), _proxyModel->filterRole(), filter, -1, flags);
		QList<QStandardItem*> items;
		for (int i = 0; i < indexes.size(); ++i) {
			items.append(_libraryModel->itemFromIndex(indexes.at(i)));
		}
		for (QStandardItem *item : items) {
			item->setData(true, Miam::DF_Highlighted);
			QStandardItem *parent = item->parent();
			// For every item marked, mark also the top level item
			while (parent != NULL) {
				parent->setData(true, Miam::DF_Highlighted);
				if (parent->parent() == NULL) {
					lettersToHighlight << parent->data(Miam::DF_NormalizedString).toString().toUpper().at(0);
				}
				parent = parent->parent();
			}
		}
	}
	_jumpToWidget->highlightLetters(lettersToHighlight);
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
	if (_searchBar) {
		_searchBar->setText(QString());
	}
	SqlDatabase::instance()->load();
	this->highlightMatchingText(QString());
}

/** Find index from current letter then scrolls to it. */
void LibraryTreeView::jumpTo(const QString &letter)
{
	QStandardItem *item = _libraryModel->letterItem(letter);
	if (item) {
		this->scrollTo(_proxyModel->mapFromSource(item->index()), PositionAtTop);
	}
}

/** Reload covers when one has changed cover size in options. */
void LibraryTreeView::reloadCovers()
{
	_itemDelegate->updateCoverSize();
	this->viewport()->update();
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
		this->verticalScrollBar()->setValue(0);
	}
	_libraryModel->reset();
}

void LibraryTreeView::endPopulateTree()
{
	_proxyModel->sort(0);
	_proxyModel->setDynamicSortFilter(true);
	_circleProgressBar->hide();
	_circleProgressBar->setValue(0);
	//_libraryModel->clearCache();
}
