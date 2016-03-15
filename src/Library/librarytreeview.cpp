#include "librarytreeview.h"

#include <library/jumptowidget.h>
#include <cover.h>
#include <filehelper.h>
#include <settings.h>
#include <settingsprivate.h>
#include "libraryfilterproxymodel.h"
#include "libraryorderdialog.h"
#include "libraryscrollbar.h"

#include <functional>

#include <QtDebug>

LibraryTreeView::LibraryTreeView(QWidget *parent)
	: TreeView(parent)
	, _libraryModel(new LibraryItemModel(this))
	, _jumpToWidget(new JumpToWidget(this))
	, properties(new QMenu(this))
{
	auto settingsPrivate = SettingsPrivate::instance();
	_proxyModel = _libraryModel->proxy();
	_proxyModel->setHeaderData(0, Qt::Horizontal, settingsPrivate->font(SettingsPrivate::FF_Menu), Qt::FontRole);
	_delegate = new LibraryItemDelegate(this, _proxyModel);

	this->setItemDelegate(_delegate);
	this->setModel(_proxyModel);
	this->setFrameShape(QFrame::NoFrame);

	auto settings = Settings::instance();
	this->setIconSize(QSize(settings->coverSizeLibraryTree(), settings->coverSizeLibraryTree()));

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

	connect(this, &LibraryTreeView::doubleClicked, this, &LibraryTreeView::appendToPlaylist);

	// Context menu
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);

	// Load album cover
	connect(this, &QTreeView::expanded, this, &LibraryTreeView::setExpandedCover);
	connect(this, &QTreeView::collapsed, this, &LibraryTreeView::removeExpandedCover);

	connect(vScrollBar, &LibraryScrollBar::aboutToDisplayItemDelegate, _delegate, &LibraryItemDelegate::displayIcon);
	connect(vScrollBar, &QAbstractSlider::valueChanged, this, [=](int) {
		QModelIndex iTop = indexAt(viewport()->rect().topLeft());
		_jumpToWidget->setCurrentLetter(_libraryModel->currentLetter(iTop));
	});
	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, &LibraryTreeView::scrollToLetter);

	connect(_proxyModel, &MiamSortFilterProxyModel::aboutToHighlightLetters, _jumpToWidget, &JumpToWidget::highlightLetters);

	connect(settingsPrivate, &SettingsPrivate::languageAboutToChange, this, [=](const QString &newLanguage) {
		QApplication::removeTranslator(&translator);
		translator.load(":/translations/library_" + newLanguage);
		QApplication::installTranslator(&translator);
	});

	connect(settingsPrivate, &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff) {
		if (ff == SettingsPrivate::FF_Library) {
			this->viewport()->update();
		}
	});

	QTimer *reloadCovers = new QTimer(this);
	reloadCovers->setSingleShot(true);
	connect(reloadCovers, &QTimer::timeout, _delegate, &LibraryItemDelegate::updateCoverSize);

	connect(settings, &Settings::viewPropertyChanged, this, &LibraryTreeView::updateViewProperty);

	// Init language
	translator.load(":/translations/library_" + settingsPrivate->language());
	QApplication::installTranslator(&translator);

	this->installEventFilter(this);
}

LibraryTreeView::~LibraryTreeView()
{
	this->disconnect();
	qDebug() << Q_FUNC_INFO;
}

const QImage *LibraryTreeView::expandedCover(AlbumItem *album) const
{
	return _expandedCovers.value(album, nullptr);
}

/** Reimplemented. */
void LibraryTreeView::findAll(const QModelIndex &index, QList<QUrl> *tracks) const
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item && item->hasChildren()) {
		for (int i = 0; i < item->rowCount(); i++) {
			// Recursive call on children
			this->findAll(index.child(i, 0), tracks);
		}
		/// FIXME
		//tracks.removeDuplicates();
	} else if (item && item->type() == Miam::IT_Track) {
		if (item->data(Miam::DF_IsRemote).toBool()) {
			tracks->append(QUrl(item->data(Miam::DF_URI).toString()));
		} else {
			tracks->append(QUrl::fromLocalFile(item->data(Miam::DF_URI).toString()));
		}
	}
}

void LibraryTreeView::removeExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && Settings::instance()->isCoverBelowTracksEnabled()) {
		AlbumItem *album = static_cast<AlbumItem*>(item);
		QImage *image = _expandedCovers.value(album);
		delete image;
		_expandedCovers.remove(album);
	}
}

void LibraryTreeView::setExpandedCover(const QModelIndex &index)
{
	QStandardItem *item = _libraryModel->itemFromIndex(_proxyModel->mapToSource(index));
	if (item->type() == Miam::IT_Album && Settings::instance()->isCoverBelowTracksEnabled()) {
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

void LibraryTreeView::updateViewProperty(Settings::ViewProperty vp, const QVariant &)
{
	switch (vp) {
	case Settings::VP_LibraryHasStarsForUnrated:
	case Settings::VP_LibraryHasStarsNextToTrack:
	case Settings::VP_LibraryHasCoverBelowTracks:
	case Settings::VP_LibraryCoverBelowTracksOpacity:
	case Settings::VP_LibraryHasCovers:
		this->viewport()->update();
		break;
	case Settings::VP_LibraryCoverSize:
		//if (!reloadCovers->isActive()) {
		//	reloadCovers->start(1000);
		//}
		this->viewport()->update();
		break;
	default:
		break;
	}
}

void LibraryTreeView::scrollToLetter(const QString &letter)
{
	_delegate->displayIcon(false);
	QStandardItem *item = _libraryModel->letterItem(letter);
	if (item) {
		this->scrollTo(_proxyModel->mapFromSource(item->index()), PositionAtTop);
	}
	_delegate->displayIcon(true);
}

void LibraryTreeView::updateSelectedTracks()
{
	/// Like the tagEditor, it's easier to proceed with complete clean/rebuild from dabatase
	qDebug() << Q_FUNC_INFO;
	/// FIXME
	//SqlDatabase().load();
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

/** Redefined to disable search in the table and trigger jumpToWidget's action. */
void LibraryTreeView::keyboardSearch(const QString &search)
{
	// If one has assigned a simple key like 'N' to 'Skip Forward' we don't actually want to skip the track
	// IMHO, it's better to trigger the JumpTo widget to 'N' section
	static QRegularExpression az("[a-z]", QRegularExpression::CaseInsensitiveOption | QRegularExpression::OptimizeOnFirstUsageOption);
	if (az.match(search).hasMatch()) {
		this->scrollToLetter(search.toUpper());
	}
}

void LibraryTreeView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp) {
		SettingsPrivate::InsertPolicy ip = SettingsPrivate::instance()->insertPolicy();
		if (ip == SettingsPrivate::IP_Albums || ip == SettingsPrivate::IP_ArtistsAlbums) {
			_delegate->displayIcon(false);
		}
	}
	TreeView::keyPressEvent(event);
}

void LibraryTreeView::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp) {
		SettingsPrivate::InsertPolicy ip = SettingsPrivate::instance()->insertPolicy();
		if (ip == SettingsPrivate::IP_Albums || ip == SettingsPrivate::IP_ArtistsAlbums) {
			_delegate->displayIcon(true);
		}
	}
	TreeView::keyReleaseEvent(event);
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
	if (_proxyModel->rowCount() == 0) {
		QPainter p(this->viewport());
		QString s = fontMetrics().elidedText(tr("No matching results were found"), Qt::ElideMiddle, viewport()->width() - _jumpToWidget->width());
		p.drawText(viewport()->rect().adjusted(0, 0, -_jumpToWidget->width(), 0), Qt::AlignCenter, s);
	} else {
		TreeView::paintEvent(event);
	}
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
	if (_libraryModel->rowCount() > 0) {
		_proxyModel->setFilterRegExp(QString());
		this->verticalScrollBar()->setValue(0);
	}
	_libraryModel->reset();
}

void LibraryTreeView::endPopulateTree()
{
	_proxyModel->sort(_proxyModel->defaultSortColumn());
	_proxyModel->setDynamicSortFilter(true);
}
