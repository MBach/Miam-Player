#include "librarytreeview.h"

#include <library/jumptowidget.h>
#include <cover.h>
#include <filehelper.h>
#include <settings.h>
#include <settingsprivate.h>
#include "deprecated/circleprogressbar.h"
#include "libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryorderdialog.h"
#include "libraryitemdelegate.h"
#include "libraryscrollbar.h"

#include <functional>

#include <QtDebug>

LibraryTreeView::LibraryTreeView(QWidget *parent)
	: TreeView(parent)
	, _libraryModel(new LibraryItemModel(parent))
	, _jumpToWidget(new JumpToWidget(this))
	, _circleProgressBar(new CircleProgressBar(this))
	, properties(new QMenu(this))
	, sendToCurrentPlaylist(new QShortcut(this))
	, openTagEditor(new QShortcut(this))
{
	auto settings = SettingsPrivate::instance();
	_proxyModel = _libraryModel->proxy();
	_proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(SettingsPrivate::FF_Menu), Qt::FontRole);
	LibraryItemDelegate *delegate = new LibraryItemDelegate(this, _proxyModel);

	this->setItemDelegate(delegate);
	this->setModel(_proxyModel);
	this->setFrameShape(QFrame::NoFrame);
	this->setIconSize(QSize(settings->coverSize(), settings->coverSize()));
	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	vScrollBar->setFrameBorder(false, false, false, true);
	this->setVerticalScrollBar(vScrollBar);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Send to the tag editor"), this);
	properties->addAction(actionSendToCurrentPlaylist);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	sortByColumn(0, Qt::AscendingOrder);
	setTextElideMode(Qt::ElideRight);

	// Context menu and shortcuts
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
	connect(sendToCurrentPlaylist, &QShortcut::activated, this, &TreeView::appendToPlaylist);
	connect(openTagEditor, &QShortcut::activated, this, &TreeView::openTagEditor);

	// Cover size
	connect(this, &LibraryTreeView::aboutToUpdateCoverSize, delegate, &LibraryItemDelegate::updateCoverSize);

	// Load album cover
	connect(this, &QTreeView::expanded, this, &LibraryTreeView::setExpandedCover);
	connect(this, &QTreeView::collapsed, this, &LibraryTreeView::removeExpandedCover);

	connect(vScrollBar, &LibraryScrollBar::aboutToDisplayItemDelegate, delegate, &LibraryItemDelegate::displayIcon);
	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, [=](const QString &letter) {
		delegate->displayIcon(false);
		this->jumpTo(letter);
		delegate->displayIcon(true);
	});

	connect(_proxyModel, &MiamSortFilterProxyModel::aboutToHighlightLetters, _jumpToWidget, &JumpToWidget::highlightLetters);
}

const QImage *LibraryTreeView::expandedCover(AlbumItem *album) const
{
	// proxy, etc
	//if (_expandedCovers.contains(album)) {
	//	qDebug() << _expandedCovers.value(album);
		return _expandedCovers.value(album, nullptr);
	//} else {
	//	return nullptr;
	//}
}

/** Reimplemented. */
void LibraryTreeView::findAll(const QModelIndex &index, QStringList &tracks) const
{
	//if (_itemDelegate) {
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
	//}
}

void LibraryTreeView::findMusic(const QString &text)
{
	_proxyModel->findMusic(text);
	/*if (SettingsPrivate::instance()->librarySearchMode() == SettingsPrivate::LSM_Filter) {
		this->filterLibrary(text);
	} else {
		this->highlightMatchingText(text);
	}*/
}

void LibraryTreeView::removeExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		AlbumItem *album = static_cast<AlbumItem*>(item);
		QImage *image = _expandedCovers.value(album);
		delete image;
		_expandedCovers.remove(album);
	}
}

void LibraryTreeView::setExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && SettingsPrivate::instance()->isBigCoverEnabled()) {
		AlbumItem *albumItem = static_cast<AlbumItem*>(item);
		QString coverPath = albumItem->coverPath();
		if (coverPath.isEmpty()) {
			return;
		}
		QImage *image = nullptr;
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
		_expandedCovers.insert(albumItem, image);
	}
}

void LibraryTreeView::updateSelectedTracks()
{
	/// Like the tagEditor, it's easier to proceed with complete clean/rebuild from dabatase
	qDebug() << Q_FUNC_INFO;
	SqlDatabase::instance()->load();
}

void LibraryTreeView::createConnectionsToDB()
{
	if (!this->property("connected").toBool()) {
		auto db = SqlDatabase::instance();
		db->disconnect();
		connect(db, &SqlDatabase::aboutToLoad, this, &LibraryTreeView::reset);
		connect(db, &SqlDatabase::loaded, this, &LibraryTreeView::endPopulateTree);
		connect(db, &SqlDatabase::progressChanged, _circleProgressBar, &QProgressBar::setValue);
		connect(db, &SqlDatabase::nodeExtracted, _libraryModel, &LibraryItemModel::insertNode);
		connect(db, &SqlDatabase::aboutToUpdateNode, _libraryModel, &LibraryItemModel::updateNode);
		connect(db, &SqlDatabase::aboutToCleanView, _libraryModel, &LibraryItemModel::cleanDanglingNodes);
		db->load();
		this->setProperty("connected", true);
	}
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		for (QAction *action : properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
			action->setFont(SettingsPrivate::instance()->font(SettingsPrivate::FF_Menu));
		}
		if (item->type() != Miam::IT_Separator) {
			properties->exec(event->globalPos());
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
		///XXX: magic number
		_jumpToWidget->move(frameGeometry().right() - 22 - wVerticalScrollBar, header()->height());
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, header()->height());
	}
	TreeView::paintEvent(event);
	///XXX: analyze performance?
	QModelIndex iTop = indexAt(viewport()->rect().topLeft());
	_jumpToWidget->setCurrentLetter(_libraryModel->currentLetter(iTop));
}

/** Recursive count for leaves only. */
int LibraryTreeView::count(const QModelIndex &index) const
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item) {
		int tmp = 0;
		for (int i = 0; i < item->rowCount(); i++) {
			tmp += count(index.child(i, 0));
		}
		return (tmp == 0) ? 1 : tmp;
	} else {
		return 0;
	}
}

/** Reimplemented. */
int LibraryTreeView::countAll(const QModelIndexList &indexes) const
{
	int c = 0;
	for (QModelIndex index : indexes) {
		c += this->count(index);
	}
	qDebug() << Q_FUNC_INFO << c;
	return c;
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
	/// FIXME
	/*if (_searchBar) {
		_searchBar->setText(QString());
	}*/
	SqlDatabase::instance()->load();
	_proxyModel->highlightMatchingText(QString());
	//this->highlightMatchingText(QString());
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
/*void LibraryTreeView::setCoverSize(int)
{
	if (itemDelegate()) {
		//_itemDelegate->setCoverSize(coverSize);
		this->viewport()->update();
	}
}*/

/** Reimplemented. */
void LibraryTreeView::reset()
{
	if (sender() == nullptr) {
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
