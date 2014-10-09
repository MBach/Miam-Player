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
#include "../library/libraryitemdelegate.h"
#include "../library/libraryorderdialog.h"
#include "jumptowidget.h"
#include "libraryfilterproxymodel.h"
#include "libraryitemdelegate.h"
#include "libraryscrollbar.h"
#include "pluginmanager.h"

#include <filehelper.h>

LibraryTreeView::LibraryTreeView(QWidget *parent) :
	TreeView(parent), _libraryModel(new QStandardItemModel(parent)), _db(NULL)
{
	_libraryModel->setColumnCount(1);

	int iconSize = SettingsPrivate::getInstance()->coverSize();
	this->setFrameShape(QFrame::NoFrame);
	this->setIconSize(QSize(iconSize, iconSize));

	proxyModel = new LibraryFilterProxyModel(this);
	proxyModel->setSourceModel(_libraryModel);
	proxyModel->setTopLevelItems(&_topLevelItems);
	_itemDelegate = new LibraryItemDelegate(this, proxyModel);
	this->setItemDelegate(_itemDelegate);

	_timer = new QTimer(this);
	_timer->setTimerType(Qt::PreciseTimer);
	_timer->setInterval(10);

	circleProgressBar = new CircleProgressBar(this);
	circleProgressBar->setTransparentCenter(true);

	QAction *actionSendToCurrentPlaylist = new QAction(tr("Send to the current playlist"), this);
	QAction *actionOpenTagEditor = new QAction(tr("Send to the tag editor"), this);
	properties = new QMenu(this);
	properties->addAction(actionSendToCurrentPlaylist);
	properties->addSeparator();
	properties->addAction(actionOpenTagEditor);

	sendToCurrentPlaylist = new QShortcut(this);
	openTagEditor = new QShortcut(this);

	connect(this, &QTreeView::doubleClicked, [=] (const QModelIndex &) { appendToPlaylist(); });
	connect(proxyModel, &LibraryFilterProxyModel::aboutToHighlight, this, [=](const QModelIndex &index, bool b) {
		if (!SettingsPrivate::getInstance()->isSearchAndExcludeLibrary()) {
			if (QStandardItem *item = _libraryModel->itemFromIndex(index)) {
				if (b) {
					//qDebug() << item->text();
				}
				item->setData(b, DF_Highlighted);
			}
		}
	});

	// Context menu and shortcuts
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	connect(sendToCurrentPlaylist, &QShortcut::activated, this, &TreeView::appendToPlaylist);
	connect(actionOpenTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
	connect(openTagEditor, &QShortcut::activated, this, &TreeView::openTagEditor);

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
	// Special item "Various" (on top) has no Normalized String
	if (iTop.data(DF_ItemType).toInt() == IT_Letter && iTop.data(DF_NormalizedString).toString().isEmpty()) {
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
		QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(index));
		if (item && item->hasChildren()) {
			for (int i = 0; i < item->rowCount(); i++) {
				// Recursive call on children
				this->findAll(index.child(i, 0), tracks);
			}
			tracks.removeDuplicates();
		} else if (item && item->data(DF_ItemType).toInt() == IT_Track) {
			tracks.append(item->data(DF_AbsFilePath).toString());
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
	SettingsPrivate *settings = SettingsPrivate::getInstance();

	proxyModel->setHeaderData(0, Qt::Horizontal, settings->font(SettingsPrivate::FF_Menu), Qt::FontRole);
	this->setModel(proxyModel);

	QObjectList objetsToExtend = QObjectList() << properties << this;
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
	connect(_db, &SqlDatabase::coverWasUpdated, this, &LibraryTreeView::updateCover);
	//connect(_db, &SqlDatabase::modelAboutToBeReset, this, &LibraryTreeView::reset);
	//connect(_db, &SqlDatabase::modelReset, this, &LibraryTreeView::endPopulateTree);
	connect(_db, &SqlDatabase::progressChanged, circleProgressBar, &QProgressBar::setValue);
	connect(_db, &SqlDatabase::trackExtracted, this, &LibraryTreeView::insertTrack);
}

/** Redefined to display a small context menu in the view. */
void LibraryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(this->indexAt(event->pos())));
	if (item) {
		foreach (QAction *action, properties->actions()) {
			action->setText(QApplication::translate("LibraryTreeView", action->text().toStdString().data()));
			action->setFont(SettingsPrivate::getInstance()->font(SettingsPrivate::FF_Menu));
		}
		if (item->data(DF_ItemType).toInt() != IT_Letter) {
			properties->exec(event->globalPos());
		}
	}
}

void LibraryTreeView::drawBranches(QPainter *painter, const QRect &r, const QModelIndex &proxyIndex) const
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	if (settings->isBigCoverEnabled()) {
		QModelIndex index2 = proxyIndex;
		QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(proxyIndex));
		//if (item->data(Type).toInt() == Track) {
		//	item = item->parent();
		//	index2 = proxyIndex.parent();
		//}
		//QRect r2 = visualRect(index2);
		if (item && item->data(DF_ItemType).toInt() == IT_Album && isExpanded(index2)) {
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
				QImage img = this->blurred(leftBorder.toImage(), leftBorder.rect(), 10, false);
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

void LibraryTreeView::drawRow(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	/// FIXME
	/*if (index.data(Type).toInt() == Track) {

	} else {

	}*/
	TreeView::drawRow(painter, option, index);
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
		_jumpToWidget->move(frameGeometry().right() - 19 - wVerticalScrollBar, header()->height());
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, header()->height());
	}
	TreeView::paintEvent(event);
}

void LibraryTreeView::bindCoverToAlbum(QStandardItem *itemAlbum, const QString &album, const QString &absFilePath)
{
	QSqlQuery internalCover("SELECT DISTINCT album FROM tracks WHERE album = ? AND internalCover = 1", _db->database());
	internalCover.addBindValue(album);
	if (!_db->database().isOpen()) {
		_db->database().open();
	}
	internalCover.exec();
	if (internalCover.next()) {
		itemAlbum->setData(absFilePath, DF_CoverPath);
	} else {
		QSqlQuery externalCover("SELECT DISTINCT coverAbsPath FROM tracks WHERE album = ?", _db->database());
		externalCover.addBindValue(album);
		externalCover.exec();
		if (externalCover.next()) {
			itemAlbum->setData(externalCover.record().value(0).toString(), DF_CoverPath);
		}
	}
}

// Thanks StackOverflow for this algorithm (works like a charm without any changes)
QImage LibraryTreeView::blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly) const
{
	int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

	QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	int r1 = rect.top();
	int r2 = rect.bottom();
	int c1 = rect.left();
	int c2 = rect.right();

	int bpl = result.bytesPerLine();
	int rgba[4];
	unsigned char* p;

	int i1 = 0;
	int i2 = 3;

	if (alphaOnly)
		i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

	for (int col = c1; col <= c2; col++) {
		p = result.scanLine(r1) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += bpl;
		for (int j = r1; j < r2; j++, p += bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = result.scanLine(row) + c1 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += 4;
		for (int j = c1; j < c2; j++, p += 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int col = c1; col <= c2; col++) {
		p = result.scanLine(r2) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= bpl;
		for (int j = r1; j < r2; j++, p -= bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = result.scanLine(row) + c2 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= 4;
		for (int j = c1; j < c2; j++, p -= 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	return result;
}

/** Recursive count for leaves only. */
int LibraryTreeView::count(const QModelIndex &index) const
{
	if (_itemDelegate) {
		QStandardItem *item = _libraryModel->itemFromIndex(proxyModel->mapToSource(index));
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
			itemLetter->setData(IT_Letter, DF_ItemType);
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

void LibraryTreeView::updateCover(const QFileInfo &coverFileInfo)
{
	QSqlQuery externalCover("SELECT DISTINCT album FROM tracks WHERE path = ?", _db->database());
	externalCover.addBindValue(QDir::toNativeSeparators(coverFileInfo.absolutePath()));
	if (!_db->database().isOpen()) {
		_db->database().open();
	}
	externalCover.exec();
	if (externalCover.next()) {
		QString album = externalCover.record().value(0).toString();
		QString alb = album.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
		QStandardItem *itemAlbum = _albums2.value(alb);
		if (itemAlbum) {
			itemAlbum->setData(coverFileInfo.absoluteFilePath(), DF_CoverPath);
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
	_db->load();
}

/** Reduces the size of the library when the user is typing text. */
void LibraryTreeView::filterLibrary(const QString &filter)
{
	if (filter.isEmpty()) {
		proxyModel->setFilterRegExp(QRegExp());
		proxyModel->sort(0, proxyModel->sortOrder());
	} else {
		bool needToSortAgain = false;
		if (proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
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
		proxyModel->setFilterRegExp(QString());
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
	switch (SettingsPrivate::getInstance()->value("insertPolicy").toInt()) {
	case IT_Artist:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists \\ Albums"));
		break;
	case IT_Album:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Albums"));
		break;
	case IT_ArtistAlbum:
		_libraryModel->horizontalHeaderItem(0)->setText(tr("  Artists – Albums"));
		break;
	case IT_Year:
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

void LibraryTreeView::insertTrack(const TrackDAO &t)
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();

	QString theArtist = t.artistAlbum().isEmpty() ? t.artist() : t.artistAlbum();
	if (settings->isLibraryFilteredByArticles()) {
		QStringList articles = settings->libraryFilteredByArticles();
		//qDebug() << articles;
		foreach (QString article, articles) {
			if (theArtist.startsWith(article + " ", Qt::CaseInsensitive)) {
				QString reorderedName = theArtist.remove(QRegularExpression("^" + article + " ", QRegularExpression::CaseInsensitiveOption)).append(", ").append(article);
				//qDebug() << theArtist << reorderedName;
				theArtist = reorderedName;
				break;
			}
		}
	}

	QStandardItem *itemArtist = NULL;
	QStandardItem *itemAlbum = NULL;
	QStandardItem *itemTrack = NULL;

	QString art = t.artist().normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
	QString theArtistNorm = theArtist.normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();
	QString alb = t.album().normalized(QString::NormalizationForm_KD).remove(QRegularExpression("[^\\w ]")).trimmed();

	static bool existingArtist = true;
	switch (settings->value("insertPolicy").toInt()) {
	case IT_Artist: {
		// Level 1
		if (_artists.contains(theArtist.toLower())) {
			itemArtist = _artists.value(theArtist.toLower());
			existingArtist = true;
		} else {
			itemArtist = new QStandardItem(theArtist);
			itemArtist->setData(IT_Artist, DF_ItemType);
			QStandardItem *letter = NULL;
			if (settings->isLibraryFilteredByArticles()) {
				itemArtist->setData(theArtistNorm, DF_NormalizedString);
				letter = this->insertLetter(theArtistNorm);
			} else {
				itemArtist->setData(art, DF_NormalizedString);
				letter = this->insertLetter(art);
			}
			_artists.insert(theArtist.toLower(), itemArtist);
			_libraryModel->invisibleRootItem()->appendRow(itemArtist);

			if (letter) {
				_topLevelItems.insert(letter->index(), itemArtist->index());
			}
			existingArtist = false;
		}
		// Level 2
		if (existingArtist && _albums.contains(QPair<QStandardItem*, QString>(itemArtist, alb))) {
			itemAlbum = _albums.value(QPair<QStandardItem*, QString>(itemArtist, alb));
		} else {
			itemAlbum = new QStandardItem(t.album());
			itemAlbum->setData(IT_Album, DF_ItemType);
			itemAlbum->setData(alb, DF_NormalizedString);
			itemAlbum->setData(t.year(), DF_Year);
			this->bindCoverToAlbum(itemAlbum, t.album(), t.url());
			_albums.insert(QPair<QStandardItem *, QString>(itemArtist, alb), itemAlbum);
			_albums2.insert(alb, itemAlbum);
			itemArtist->appendRow(itemAlbum);
		}
		// Level 3 (option)
		int disc = t.disc().toInt();
		if (disc > 0 && !_discNumbers.contains(QPair<QStandardItem*, int>(itemAlbum, disc))) {
			QStandardItem *itemDiscNumber = new QStandardItem(t.disc());
			itemDiscNumber->setData(IT_Disc, DF_ItemType);
			itemDiscNumber->setData(t.disc(), DF_DiscNumber);
			//qDebug() << title << discNumber;
			_discNumbers.insert(QPair<QStandardItem *, int>(itemAlbum, disc), itemDiscNumber);
			itemAlbum->appendRow(itemDiscNumber);
		}
		// Level 3
		if (t.artistAlbum().isEmpty() || QString::compare(t.artist(), t.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(t.title());
		} else {
			itemTrack = new QStandardItem(t.title() + " (" + t.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	}
	case IT_Album:
		// Level 1
		if (_albums2.contains(alb)) {
			itemAlbum = _albums2.value(alb);
		} else {
			itemAlbum = new QStandardItem(t.album());
			itemAlbum->setData(IT_Album, DF_ItemType);
			itemAlbum->setData(alb, DF_NormalizedString);
			itemAlbum->setData(t.year(), DF_Year);
			this->bindCoverToAlbum(itemAlbum, t.album(), t.url());
			_albums2.insert(alb, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			QStandardItem *letter = this->insertLetter(alb);
			if (letter) {
				_topLevelItems.insert(letter->index(), itemAlbum->index());
			}
		}
		// Level 2
		itemTrack = new QStandardItem(t.title());
		itemAlbum->appendRow(itemTrack);
		break;
	case IT_ArtistAlbum:
		// Level 1
		if (_albums2.contains(theArtistNorm + alb)) {
			itemAlbum = _albums2.value(theArtistNorm + alb);
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + t.album());
			itemAlbum->setData(IT_Album, DF_ItemType);
			itemAlbum->setData(theArtistNorm + alb, DF_NormalizedString);
			itemAlbum->setData(t.year(), DF_Year);
			this->bindCoverToAlbum(itemAlbum, t.album(), t.url());
			_albums2.insert(theArtistNorm + alb, itemAlbum);
			_libraryModel->invisibleRootItem()->appendRow(itemAlbum);
			QStandardItem *letter = this->insertLetter(theArtistNorm + alb);
			if (letter) {
				_topLevelItems.insert(letter->index(), itemAlbum->index());
			}
		}
		// Level 2
		if (t.artistAlbum().isEmpty() || QString::compare(t.artist(), t.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(t.title());
		} else {
			itemTrack = new QStandardItem(t.title() + " (" + t.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	case IT_Year:{
		// Level 1
		QStandardItem *itemYear = NULL;
		if (_years.contains(t.year().toInt())) {
			itemYear = _years.value(t.year().toInt());
		} else {
			if (t.year().toInt() > 0) {
				itemYear = new QStandardItem(t.year());
			} else {
				itemYear = new QStandardItem(tr("Unknown"));
			}
			itemYear->setData(IT_Year, DF_ItemType);
			itemYear->setData(t.year(), DF_NormalizedString);
			_years.insert(t.year().toInt(), itemYear);
			_libraryModel->invisibleRootItem()->appendRow(itemYear);
		}
		// Level 2
		if (_albums2.contains(theArtist + alb)) {
			itemAlbum = _albums2.value(theArtist + alb);
		} else {
			itemAlbum = new QStandardItem(theArtist + " – " + t.album());
			itemAlbum->setData(IT_Album, DF_ItemType);
			itemAlbum->setData(art.append("|").append(alb), DF_NormalizedString);
			itemAlbum->setData(t.year(), DF_Year);
			this->bindCoverToAlbum(itemAlbum, t.album(), t.url());
			_albums2.insert(theArtist + alb, itemAlbum);
			itemYear->appendRow(itemAlbum);
		}
		// Level 3
		if (t.artistAlbum().isEmpty() || QString::compare(t.artist(), t.artistAlbum()) == 0) {
			itemTrack = new QStandardItem(t.title());
		} else {
			itemTrack = new QStandardItem(t.title() + " (" + t.artist() + ")");
		}
		itemAlbum->appendRow(itemTrack);
		break;
	}
	}
	/// XXX: Is it necessary to create subclasses of QStandardItem for item->type()?
	/// YES! (07/14)
	// itemTrack always exists
	itemTrack->setData(IT_Track, DF_ItemType);
	itemTrack->setData(t.url(), DF_AbsFilePath);
	itemTrack->setData(t.trackNumber(), DF_TrackNumber);
	itemTrack->setData(t.disc(), DF_DiscNumber);
}
