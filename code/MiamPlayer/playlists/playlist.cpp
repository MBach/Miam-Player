#include "playlist.h"

#include <QApplication>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QTime>

#include "../columnutils.h"
#include "settings.h"
#include "../library/librarytreeview.h"
#include "tabplaylist.h"
#include "playlistheaderview.h"
#include "playlistitemdelegate.h"

#include <QItemSelection>
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

	// Select only by rows, not cell by cell
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setShowGrid(false);

	// Init child members
	verticalHeader()->hide();
	this->setHorizontalHeader(new PlaylistHeaderView(this));

	connect(this, &QTableView::doubleClicked, [=](const QModelIndex &track) {
		// Prevent the signal "currentMediaChanged" for being emitted twice
		_mediaPlayer.data()->blockSignals(true);
		_mediaPlayer.data()->setPlaylist(_playlistModel->mediaPlaylist());
		_mediaPlayer.data()->blockSignals(false);
		_mediaPlayer.data()->playlist()->setCurrentIndex(track.row());
		_mediaPlayer.data()->play();
	});

	// Link core multimedia actions
	connect(_mediaPlayer.data(), &QMediaPlayer::mediaStatusChanged, [=] (QMediaPlayer::MediaStatus status) {
		if (status == QMediaPlayer::EndOfMedia) {
			_mediaPlayer.data()->skipForward();
		}
	});
	connect(_mediaPlayer.data(), &QMediaPlayer::currentMediaChanged, [=] () {
		this->update();
	});

	// Context menu on tracks
	_trackProperties = new QMenu(this);
	QAction *removeFromCurrentPlaylist = _trackProperties->addAction(tr("Remove from playlist"));
    connect(removeFromCurrentPlaylist, &QAction::triggered, this, &Playlist::removeSelectedTracks);

	// Set row height
	verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(Settings::PLAYLIST)).height());

	connect(mediaPlaylist(), &QMediaPlaylist::loaded, [=] () {
		for (int i = 0; i < mediaPlaylist()->mediaCount(); i++) {
			_playlistModel->insertMedia(i, mediaPlaylist()->media(i));
		}
	});

	// No pity: marks everything as a dirty region
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection & selected, const QItemSelection &) {
		this->setDirtyRegion(QRegion(this->viewport()->rect()));
		_previouslySelectedRows = selected.indexes();
		qDebug() << "previouslySelectedRows" << _previouslySelectedRows;
	});

	QList<QScrollBar*> scrollBars = QList<QScrollBar*>() << horizontalScrollBar() << verticalScrollBar();
	foreach (QScrollBar *scrollBar, scrollBars) {
		connect(scrollBar, &QScrollBar::sliderPressed, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderMoved, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderReleased, [=]() { viewport()->update(); });
	}
}

void Playlist::insertMedias(int rowIndex, const QList<QMediaContent> &medias)
{
	_playlistModel->insertMedias(rowIndex, medias);
	this->resizeColumnToContents(TRACK_NUMBER);
	this->resizeColumnToContents(RATINGS);
	this->resizeColumnToContents(YEAR);
}

void Playlist::insertMedias(int rowIndex, const QStringList &tracks)
{
	QList<QMediaContent> medias;
	foreach (QString track, tracks) {
		medias.append(QMediaContent(QUrl::fromLocalFile(track)));
	}
	// If the track needs to be appended at the end
	if (rowIndex == -1) {
		rowIndex = _playlistModel->rowCount();
	}
	this->insertMedias(rowIndex, medias);
}

QSize Playlist::minimumSizeHint() const
{
	int width = 0;
	Settings *settings = Settings::getInstance();
	QFont font = settings->font(Settings::PLAYLIST);
	QFontMetrics fm(font);
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
	repaint();
	delete _dropDownIndex;
	_dropDownIndex = NULL;
}

/** Redefined to be able to move tracks between playlists or internally. */
void Playlist::dropEvent(QDropEvent *event)
{
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
	if (!(event->buttons() & Qt::LeftButton))
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

#include <QPaintEngine>

/** Redefined to display a thin line to help user for dropping tracks. */
void Playlist::paintEvent(QPaintEvent *event)
{
	QTableView::paintEvent(event);
	if (_dropDownIndex) {
		// Where to draw the indicator line
		int rowDest = _dropDownIndex->row() >= 0 ? _dropDownIndex->row() : _playlistModel->rowCount();
		int height = this->rowHeight(0);
		/// TODO computes color from user defined settings
		QPainter p(viewport());
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
	for (int i = indexes.size() - 1; i >= 0; i--) {
		int row = indexes.at(i).row();
		_playlistModel->removeRow(row);
	}
}
