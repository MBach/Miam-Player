#include "playlist.h"

#include <QApplication>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QTime>

#include <fileref.h>
#include <tag.h>

#include "settings.h"
#include "nofocusitemdelegate.h"
#include "library/librarytreeview.h"
#include "tabplaylist.h"

#include <QtDebug>

Playlist::Playlist(QWidget *parent, QMediaPlayer *mediaPlayer) :
	QTableView(parent), _mediaPlayer(mediaPlayer)
{
	qMediaPlaylist = new QMediaPlaylist(this);
	_playlistModel = new PlaylistModel(qMediaPlaylist);
    _mediaPlayer->setPlaylist(qMediaPlaylist);

	this->setModel(_playlistModel);

	Settings *settings = Settings::getInstance();
	// Init direct members
	this->setAcceptDrops(true);
	this->setAlternatingRowColors(settings->colorsAlternateBG());

	this->setColumnHidden(5, true);
	this->setColumnHidden(6, true);
	this->setDragEnabled(true);
	this->setDragDropMode(QAbstractItemView::DragDrop);
	this->setItemDelegate(new NoFocusItemDelegate(this));
	this->setHorizontalHeader(new QHeaderView(Qt::Horizontal, this));
	// Select only one row, not cell by cell
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setShowGrid(false);
	this->setStyleSheet(settings->styleSheet(this));

	// Init child members
	verticalScrollBar()->setStyleSheet(settings->styleSheet(this->verticalScrollBar()));
	verticalHeader()->setVisible(false);
	horizontalHeader()->setStyleSheet(settings->styleSheet(horizontalHeader()));
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	horizontalHeader()->setHighlightSections(false);
	/// FIXME Qt5
	//horizontalHeader()->setMovable(true);
	//horizontalHeader()->setResizeMode(QHeaderView::Fixed);

	// Context menu on tracks
	trackProperties = new QMenu(this);
	QAction *removeFromCurrentPlaylist = trackProperties->addAction(tr("Remove from playlist"));
    connect(removeFromCurrentPlaylist, &QAction::triggered, this, &Playlist::removeSelectedTracks);

	// Context menu on header of columns
	columns = new QMenu(this);

	//TabPlaylist *tabPlaylist = qobject_cast<TabPlaylist*>(parent);
	//connect(this, &QTableView::doubleClicked, tabPlaylist, &TabPlaylist::changeTrack);
	//connect(this, &QTableView::doubleClicked, this, &Playlist::changeTrack);
	connect(this, &QTableView::doubleClicked, [=] (const QModelIndex &index) { qMediaPlaylist->setCurrentIndex(index.row()); });
	connect(qMediaPlaylist, &QMediaPlaylist::currentIndexChanged, this, &Playlist::changeTrack);
	connect(columns, SIGNAL(triggered(QAction*)), this, SLOT(toggleSelectedColumn(QAction*)));

	// Link this playlist with the Settings instance to change fonts at runtime
    connect(Settings::getInstance(), &Settings::currentFontChanged, this, &Playlist::highlightCurrentTrack);

	// Hide the selected column in context menu
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &Playlist::showColumnsMenu);
    connect(horizontalHeader(), &QHeaderView::sectionMoved, this, &Playlist::saveColumnsState);

	//connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(countSelectedItems(QItemSelection,QItemSelection)));
	//connect(playlistModel(), SIGNAL(layoutChanged()), this, SLOT(update()));

	connect(qMediaPlaylist, &QMediaPlaylist::mediaInserted, _playlistModel, &PlaylistModel::insertMedia);
}

void Playlist::init()
{
	QStringList labels = (QStringList() << "#" << tr("Title") << tr("Album") << tr("Length") << tr("Artist") << tr("Rating") << tr("Year"));

	_playlistModel->setColumnCount(labels.count());

	// Initialize values for the Header (label and horizontal resize mode)
	for (int i = 0; i < labels.size(); i++) {
		_playlistModel->setHeaderData(i, Qt::Horizontal, labels.at(i), Qt::DisplayRole);
	}

	for (int i = 0; i < _playlistModel->columnCount(); i++) {
		QString label = labels.at(i);

		// Match actions with columns using index of labels
		QAction *actionColumn = new QAction(label, this);
		actionColumn->setData(i);
		actionColumn->setEnabled(actionColumn->text() != tr("Title"));
		actionColumn->setCheckable(true);
		actionColumn->setChecked(!isColumnHidden(i));

		// Then populate the context menu
		columns->addAction(actionColumn);
	}

	// Load columns state
	horizontalHeader()->restoreState(Settings::getInstance()->value("playlistColumnsState").toByteArray());
	for (int i = 0; i < horizontalHeader()->count(); i++) {
		bool hidden = horizontalHeader()->isSectionHidden(i);
		setColumnHidden(i, hidden);
		columns->actions().at(i)->setChecked(!hidden);
	}
}

/** Display a context menu with the state of all columns. */
void Playlist::showColumnsMenu(const QPoint &pos)
{
	columns->exec(mapToGlobal(pos));
}

/** Retranslate header columns. */
void Playlist::retranslateUi()
{
	const QStringList labels = (QStringList() << "#" << tr("Title") << tr("Album") << tr("Length") << tr("Artist") << tr("Rating") << tr("Year"));

	// Initialize values for the Header (label and horizontal resize mode)
	for (int i = 0; i < labels.size(); i++) {
		_playlistModel->setHeaderData(i, Qt::Horizontal, labels.at(i), Qt::DisplayRole);
		columns->actions().at(i)->setText(labels.at(i));
	}
}

/** Redefined to display a small context menu in the view. */
void Playlist::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QStandardItem *item = _playlistModel->itemFromIndex(index);
	if (item != NULL) {
		trackProperties->exec(event->globalPos());
	}
}

void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
	// If the source of the drag and drop is another application
	if (event->source() == NULL) {
		event->ignore();
	} else {
		event->acceptProposedAction();
	}
}

void Playlist::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void Playlist::dropEvent(QDropEvent *event)
{
	QObject *source = event->source();
	if (TreeView *view = qobject_cast<TreeView*>(source)) {
		int row = this->indexAt(event->pos()).row();
		view->sendToPlaylist(this, row-1);
	} else if (Playlist *currentPlaylist = qobject_cast<Playlist*>(source)) {
		if (currentPlaylist == this) {
			qDebug() << "internal move";
			QModelIndexList list = this->selectionModel()->selectedRows();
			int destChild = this->indexAt(event->pos()).row();
			//this->playlistModel()->move(list, destChild);
		}
	}
}

void Playlist::mouseMoveEvent(QMouseEvent *event)
{
	if (!_selected && state() == NoState) {
		this->setState(DragSelectingState);
	}
	QTableView::mouseMoveEvent(event);
}

void Playlist::mousePressEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	//_selected = selectionModel()->isSelected(index);
	QTableView::mousePressEvent(event);
}

void Playlist::resizeEvent(QResizeEvent *)
{
	/// XXX: need to be improved to avoid flickering
	this->resizeColumns();
}

void Playlist::changeTrack(int i)
{
    qDebug() << "Playlist::changeTrack =" << i;
	Q_UNUSED(i)
    //_mediaPlayer->play();
    qDebug() << "volume" << _mediaPlayer->volume();
    qDebug() << "media" << qMediaPlaylist->media(i).canonicalUrl();
}

void Playlist::resizeColumns()
{
	int visibleRatio = 0;
	int resizableArea = size().width() - 4;
	if (verticalScrollBar()->isVisible()) {
		resizableArea -= verticalScrollBar()->size().width();
	}

	// Test: 0 = Fixed, n>0 = real ratio for each column
	const QList<int> ratios(QList<int>() << 0 << 5 << 4 << 1 << 3 << 0 << 0);

	// Resize fixed columns first, and then compute the remaining width
	for (int c = 0; c < _playlistModel->columnCount(); c++) {
		if (!isColumnHidden(c)) {
			int ratio = ratios.at(c);
			// Fixed column
			if (ratio == 0) {
				this->resizeColumnToContents(c);
				resizableArea -= columnWidth(c) - 1;
			}
			visibleRatio += ratio;
		}
	}
	for (int c = 0; c < _playlistModel->columnCount(); c++) {
		int ratio = ratios.at(c);
		// Resizable column
		if (ratio != 0) {
			int s = resizableArea * ratio / visibleRatio ;
			if (!isColumnHidden(c)) {
				this->setColumnWidth(c, s);
			}
		}
	}
}

void Playlist::countSelectedItems(const QItemSelection &, const QItemSelection &)
{
	this->countSelectedItems();
}

void Playlist::countSelectedItems()
{
	//emit selectedTracks(this->selectionModel()->selectedRows().size());
}

/** Toggle the selected column from the context menu. */
void Playlist::toggleSelectedColumn(QAction *action)
{
	int columnIndex = action->data().toInt();
	this->setColumnHidden(columnIndex, !isColumnHidden(columnIndex));
	this->resizeColumns();
	this->saveColumnsState();
}

/** Save state when one checks or moves a column. */
void Playlist::saveColumnsState(int /*logicalIndex*/, int /*oldVisualIndex*/, int /*newVisualIndex*/)
{
	// The pair "playlistColumnsState" is only used in this class, so there's no need to create specific getter and setter
	Settings::getInstance()->setValue("playlistColumnsState", horizontalHeader()->saveState());
}

/** Move selected tracks downward. */
void Playlist::moveTracksDown()
{
	/*if (currentItem()) {
		int currentRow = currentItem()->row();
		if (currentRow < rowCount()-1) {
			for (int c=0; c < columnCount(); c++) {
				QTableWidgetItem *currentItem = takeItem(currentRow, c);
				QTableWidgetItem *nextItem = takeItem(currentRow+1, c);
				setItem(currentRow, c, nextItem);
				setItem(currentRow+1, c, currentItem);
			}
			sources.swap(currentRow, currentRow+1);
			this->setCurrentIndex(_playlistModel()->index(currentRow+1, 0));
			if (currentRow == track) {
				track++;
			}
		}
	}*/
}

/** Move selected tracks upward. */
void Playlist::moveTracksUp(int i)
{
	/*QList<QTableWidgetItem *> selection = selectedItems();
	int prev = -1;
	for (int k = selection.length() - 1; k >= 0; k--) {
		int current = selection.at(k)->row();
		if (current != prev) {
			QTableWidgetItem *item = takeItem(current, 0);
			QTableWidgetItem *itemBelow = takeItem(current+1, 0);
			setItem(current, 0, itemBelow);
			setItem(current+1, 0, item);
			prev = current;
		}
	}*/

	/*if (this->selectionModel()->hasSelection()) {
		int currentRow = currentItem()->row();
		if (currentRow > 0) {
			for (int c=0; c < columnCount(); c++) {
				QTableWidgetItem *currentItem = takeItem(currentRow, c);
				QTableWidgetItem *previousItem = takeItem(currentRow-1, c);
				setItem(currentRow, c, previousItem);
				setItem(currentRow-1, c, currentItem);
			}
			sources.swap(currentRow, currentRow-1);
			setCurrentIndex(_playlistModel()->index(currentRow-1, 0));
			if (currentRow == track) {
				track--;
			}
		}
	}*/
}

/** Remove selected tracks from the playlist. */
void Playlist::removeSelectedTracks()
{
	QModelIndexList indexes = this->selectionModel()->selectedRows();
	for (int i = indexes.size() - 1; i >= 0; i--) {
		//playlistModel()->removeRow(indexes.at(i).row());
	}
}

/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
void Playlist::highlightCurrentTrack()
{
	QStandardItem *it;
	const QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
	/*if (playlistModel()->rowCount() > 0) {
		for (int i=0; i < playlistModel()->rowCount(); i++) {
			for (int j = 0; j < playlistModel()->columnCount(); j++) {
				it = playlistModel()->item(i, j);
				QFont itemFont = font;
				itemFont.setBold(false);
				itemFont.setItalic(false);
				it->setFont(itemFont);
				QFontMetrics fm(itemFont);
				this->setRowHeight(i, fm.height());
			}
		}
		for (int j=0; j < playlistModel()->columnCount(); j++) {
			it = playlistModel()->item(playlistModel()->activeTrack(), j);
			// If there is actually one selected track in the playlist
			if (it != NULL) {
				QFont itemFont = font;
				itemFont.setBold(true);
				itemFont.setItalic(true);
				it->setFont(itemFont);
			}
		}
	}*/
}
