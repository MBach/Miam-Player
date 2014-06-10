#include "playlist.h"

#include <QApplication>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QTime>

#include "../columnutils.h"
#include "../library/librarytreeview.h"
#include "tabplaylist.h"
#include "playlistheaderview.h"
#include "playlistitemdelegate.h"
#include "scrollbar.h"

#include <QItemSelection>
#include <QPaintEngine>
#include <QStylePainter>

#include <QtDebug>

Playlist::Playlist(QWeakPointer<MediaPlayer> mediaPlayer, QWidget *parent) :
	QTableView(parent), _mediaPlayer(mediaPlayer), _dropDownIndex(NULL), _hash(0)
{
	_playlistModel = new PlaylistModel(this);

	this->setModel(_playlistModel);

	Settings *settings = Settings::getInstance();
	// Init direct members
	this->setAcceptDrops(true);
	this->setAlternatingRowColors(settings->colorsAlternateBG());
	this->setColumnHidden(5, true);
	this->setColumnHidden(6, true);
	this->setDragDropMode(QAbstractItemView::DragDrop);
	this->setDragEnabled(true);
	this->setDropIndicatorShown(true);
	this->setEditTriggers(QTableView::SelectedClicked);
	this->setFrameShape(QFrame::NoFrame);
	this->setItemDelegate(new PlaylistItemDelegate(this));
	this->setMouseTracking(true);
	ScrollBar *hScrollBar = new ScrollBar(Qt::Vertical, this);
	hScrollBar->setFrameBorder(false, true, true, false);
	this->setHorizontalScrollBar(hScrollBar);
	this->setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));

	// Select only by rows, not cell by cell
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setShowGrid(false);

	// Init child members
	verticalHeader()->hide();
	this->setHorizontalHeader(new PlaylistHeaderView(this));

	connect(this, &QTableView::doubleClicked, this, [=](const QModelIndex &track) {
		// Prevent the signal "currentMediaChanged" for being emitted twice
		_mediaPlayer.data()->blockSignals(true);
		_mediaPlayer.data()->setPlaylist(_playlistModel->mediaPlaylist());
		_mediaPlayer.data()->blockSignals(false);
		qDebug() << "about to set current index";
		_mediaPlayer.data()->playlist()->setCurrentIndex(track.row());
		_mediaPlayer.data()->play();
	});

	// Link core multimedia actions
	connect(_mediaPlayer.data(), &MediaPlayer::mediaStatusChanged, this, [=] (QMediaPlayer::MediaStatus status) {
		if (status == QMediaPlayer::EndOfMedia) {
			_mediaPlayer.data()->skipForward();
		}
	});

	// Ensure current item in the playlist is visible when track has just changed to another one
	connect(_mediaPlayer.data(), &MediaPlayer::currentMediaChanged, this, [=] (const QMediaContent &media) {
		if (!media.isNull()) {
			int row = _mediaPlayer.data()->playlist()->currentIndex();
			this->scrollTo(_playlistModel->index(row, 0));
			this->viewport()->update();
		}
	});

	// Context menu on tracks
	/// TODO: sub menu tag -> send playlist to editor, edit in-line
	_trackProperties = new QMenu(this);
	QAction *removeFromCurrentPlaylist = _trackProperties->addAction(tr("Remove from playlist"));
	QMenu *subMenuTag = new QMenu(tr("Edit tags"), this);
	_trackProperties->addMenu(subMenuTag);
	QAction *actionEditTagsInEditor = subMenuTag->addAction(tr("in tag editor"));
	QAction *actionInlineTag = subMenuTag->addAction(tr("inline"));
	actionInlineTag->setToolTip("Not yet implemented");
	actionInlineTag->setDisabled(true);

	connect(removeFromCurrentPlaylist, &QAction::triggered, this, &Playlist::removeSelectedTracks);
	//connect(actionEditTagsInEditor, &QAction::triggered, this, &Playlist::editTagsInEditor);
	connect(actionEditTagsInEditor, &QAction::triggered, [=]() {
		QList<QUrl> selectedTracks;
		foreach (QModelIndex index, selectionModel()->selectedRows()) {
			QMediaContent mc = _playlistModel->mediaPlaylist()->media(index.row());
			selectedTracks.append(mc.canonicalUrl());
		}
		emit aboutToSendToTagEditor(selectedTracks);
	});
	/// TODO
	//connect(actionInlineTag, &QAction::triggered, this, &Playlist::editTagInline);

	// Set row height
	verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(Settings::PLAYLIST)).height());

	connect(mediaPlaylist(), &QMediaPlaylist::loaded, this, [=] () {
		for (int i = 0; i < mediaPlaylist()->mediaCount(); i++) {
			_playlistModel->insertMedia(i, mediaPlaylist()->media(i));
		}
	});

	// No pity: marks everything as a dirty region
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](const QItemSelection & selected, const QItemSelection &) {
		this->setDirtyRegion(QRegion(this->viewport()->rect()));
		_previouslySelectedRows = selected.indexes();
		emit selectionChanged(selected.isEmpty());
	});

	QList<QScrollBar*> scrollBars = QList<QScrollBar*>() << horizontalScrollBar() << verticalScrollBar();
	foreach (QScrollBar *scrollBar, scrollBars) {
		connect(scrollBar, &QScrollBar::sliderPressed, this, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderMoved, this, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderReleased, this, [=]() { viewport()->update(); });
	}
}

void Playlist::insertMedias(int rowIndex, const QList<QMediaContent> &medias)
{
	_playlistModel->insertMedias(rowIndex, medias);
	if (Settings::getInstance()->isPlaylistResizeColumns()) {
		this->resizeColumnsToContents();
	} else {
		this->resizeColumnToContents(TRACK_NUMBER);
		this->resizeColumnToContents(RATINGS);
		this->resizeColumnToContents(YEAR);
	}
}

void Playlist::insertMedias(int rowIndex, const QStringList &tracks)
{
	QList<QMediaContent> medias;
	foreach (QString track, tracks) {
		QMediaContent media(QUrl::fromLocalFile(track));
		if (!media.isNull()) {
			medias.append(media);
		}
	}
	// If the track needs to be appended at the end
	if (rowIndex == -1) {
		rowIndex = _playlistModel->rowCount();
	}
	this->insertMedias(rowIndex, medias);
}

QSize Playlist::minimumSizeHint() const
{
	QFontMetrics fm(Settings::getInstance()->font(Settings::PLAYLIST));
	int width = 0;
	for (int c = 0; c < _playlistModel->columnCount(); c++) {
		if (!isColumnHidden(c)) {
			width += fm.width(_playlistModel->headerData(c, Qt::Horizontal).toString());
		}
	}
	return QTableView::minimumSizeHint();
}

/** Redefined to display a small context menu in the view. */
void Playlist::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QStandardItem *item = _playlistModel->itemFromIndex(index);
	if (item != NULL) {
		foreach (QAction *action, _trackProperties->actions()) {
			action->setText(tr(action->text().toStdString().data()));
		}
		_trackProperties->exec(event->globalPos());
	}
}

void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
	// If the source of the drag and drop is another application, do nothing?
	if (event->source() == NULL) {
		event->ignore();
	} else {
		event->acceptProposedAction();
	}
}

void Playlist::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
	_dropDownIndex = new QModelIndex();
	// Kind of hack to keep track of position?
	*_dropDownIndex = indexAt(event->pos());
	//qDebug() << "DDI" << _dropDownIndex->row();
	//repaint();
	delete _dropDownIndex;
	_dropDownIndex = NULL;
}

/** Redefined to be able to move tracks between playlists or internally. */
void Playlist::dropEvent(QDropEvent *event)
{
	qDebug() << Q_FUNC_INFO;
	QObject *source = event->source();
	int row = this->indexAt(event->pos()).row();
	if (TreeView *view = qobject_cast<TreeView*>(source)) {
		view->insertToPlaylist(row);
	} else if (Playlist *target = qobject_cast<Playlist*>(source)) {
		// Internal drag and drop (moving tracks)
		if (target && target == this) {
			QList<QStandardItem*> rowsToHighlight = _playlistModel->internalMove(indexAt(event->pos()), selectionModel()->selectedRows());
			// Highlight rows that were just moved
			foreach (QStandardItem *item, rowsToHighlight) {
				for (int c = 0; c < _playlistModel->columnCount(); c++) {
					QModelIndex index = _playlistModel->index(item->row(), c);
					selectionModel()->select(index, QItemSelectionModel::Select);
				}
			}
		} else if (target && target != this) {
			// If the drop occurs at the end of the playlist, indexAt is invalid
			if (row == -1) {
				row = _playlistModel->rowCount();
			}
			QList<QMediaContent> medias;
			foreach (QModelIndex index, target->selectionModel()->selectedRows()) {
				medias.append(target->mediaPlaylist()->media(index.row()));
			}
			this->insertMedias(row, medias);

			// Highlight rows that were just moved
			this->clearSelection();
			for (int r = 0; r < medias.count(); r++) {
				for (int c = 0; c < _playlistModel->columnCount(); c++) {
					QModelIndex index = _playlistModel->index(row + r, c);
					selectionModel()->select(index, QItemSelectionModel::Select);
				}
			}
			if (!Settings::getInstance()->copyTracksFromPlaylist()) {
				target->removeSelectedTracks();
			}
		}
	} else if (source == NULL) {
		event->ignore();
		qDebug() << "source is null, ignore event?" << this->parent() << this->objectName();
		return;
	}
}

/** Redefined to handle escape key when editing ratings. */
void Playlist::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		// Two level escape: if there are editors or not
		if (findChildren<StarEditor*>().isEmpty()) {
			selectionModel()->clearSelection();
		} else {
			for (int row = 0; row < _playlistModel->rowCount(); row++) {
				closePersistentEditor(_playlistModel->index(row, RATINGS));
			}
		}
	}
	QTableView::keyPressEvent(event);
}

void Playlist::mouseMoveEvent(QMouseEvent *event)
{
	if (!event->buttons().testFlag(Qt::LeftButton))
		return;
	if ((event->pos() - _dragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;
	QTableView::mouseMoveEvent(event);
}

void Playlist::mousePressEvent(QMouseEvent *event)
{
	// For drag & drop
	if (event->button() == Qt::LeftButton) {
		_dragStartPosition = event->pos();
	}
	QModelIndex index = indexAt(event->pos());
	if (index.column() == RATINGS && _previouslySelectedRows.contains(index)) {
		//qDebug() << "clic on " << index;
		foreach (QModelIndex i, selectionModel()->selectedRows(RATINGS)) {
			this->openPersistentEditor(i);
		}
	} else {
		QTableView::mousePressEvent(event);
	}
}

/** Redefined to display a thin line to help user for dropping tracks. */
void Playlist::paintEvent(QPaintEvent *event)
{
	QTableView::paintEvent(event);
	QPainter p(viewport());
	p.setPen(QApplication::palette().mid().color());
	if (isLeftToRight()) {
		p.drawLine(viewport()->rect().topLeft(), viewport()->rect().bottomLeft());
	} else {
		p.drawLine(viewport()->rect().topRight(), viewport()->rect().bottomRight());
	}
	if (_dropDownIndex) {
		// Where to draw the indicator line
		int rowDest = _dropDownIndex->row() >= 0 ? _dropDownIndex->row() : _playlistModel->rowCount();
		int height = this->rowHeight(0);
		/// TODO computes color from user defined settings

		p.setPen(Qt::black);
		p.drawLine(viewport()->rect().left(), rowDest * height,
				   viewport()->rect().right(), rowDest * height);
	}
}

int Playlist::sizeHintForColumn(int column) const
{
	if (column == RATINGS) {
		return rowHeight(RATINGS) * 5;
	} else {
		return QTableView::sizeHintForColumn(column);
	}
}

void Playlist::showEvent(QShowEvent *event)
{
	resizeColumnToContents(TRACK_NUMBER);
	resizeColumnToContents(RATINGS);
	resizeColumnToContents(YEAR);
	QTableView::showEvent(event);
}

/** Move selected tracks downward. */
void Playlist::moveTracksDown()
{
	/// TODO
}

/** Move selected tracks upward. */
void Playlist::moveTracksUp()
{
	/// TODO
}

/** Remove selected tracks from the playlist. */
void Playlist::removeSelectedTracks()
{
	QModelIndexList indexes = this->selectionModel()->selectedRows();
	int firstIndex = INT_MAX;
	foreach (QModelIndex idx, indexes) {
		if (idx.row() < firstIndex) {
			firstIndex = idx.row();
		}
	}
	// Remove discontiguous rows
	for (int i = indexes.size() - 1; i >= 0; i--) {
		int row = indexes.at(i).row();
		_playlistModel->removeRow(row);
	}
	if (firstIndex < _playlistModel->rowCount()) {
		this->selectRow(firstIndex);
	} else {
		// Select the last one otherwise: it still can be possible to erase all
		this->selectRow(_playlistModel->rowCount() - 1);
	}
}
