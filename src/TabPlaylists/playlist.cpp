#include "playlist.h"

#include <QApplication>
#include <QDropEvent>
#include <QHeaderView>

#include <scrollbar.h>
#include "playlistheaderview.h"
#include "playlistitemdelegate.h"
#include <settingsprivate.h>

#include <QMimeData>
#include <QDrag>
#include <QPaintEngine>

#include <QtDebug>

Playlist::Playlist(MediaPlayer *mediaPlayer, QWidget *parent)
	: QTableView(parent)
	, _mediaPlayer(mediaPlayer)
	, _isDragging(false)
	, _hash(0)
	, _id(0)
{
	_playlistModel = new PlaylistModel(this);

	this->setModel(_playlistModel);

	SettingsPrivate *settings = SettingsPrivate::instance();
	// Init direct members
	this->setAcceptDrops(true);
	this->setAlternatingRowColors(settings->colorsAlternateBG());
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
	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));

	// Select only by rows, not cell by cell
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setShowGrid(false);

	// Init child members
	verticalHeader()->hide();
	auto headerView = new PlaylistHeaderView(this);
	this->setHorizontalHeader(headerView);
	/// XXX not working?
	//headerView->setSectionResizeMode(Playlist::COL_RATINGS, QHeaderView::Fixed);

	// Set row height
	verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(SettingsPrivate::FF_Playlist)).height());

	connect(this, &Playlist::doubleClicked, this, [=] (const QModelIndex &track) {
		_playlistModel->mediaPlaylist()->setCurrentIndex(track.row());
		mediaPlayer->setPlaylist(_playlistModel->mediaPlaylist());
		mediaPlayer->play();
	});

	// Ensure current item in the playlist is visible when track has just changed to another one
	connect(mediaPlayer, &MediaPlayer::currentMediaChanged, this, [=] (const QString &uri) {
		if (mediaPlayer->playlist() == this->mediaPlaylist() && !uri.isEmpty()) {
			int row = mediaPlaylist()->currentIndex();
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
	connect(actionEditTagsInEditor, &QAction::triggered, this, [=]() {
		QModelIndexList indexes;
		QList<QUrl> selectedTracks;
		for (QModelIndex index : selectionModel()->selectedRows()) {
			QMediaContent mc = _playlistModel->mediaPlaylist()->media(index.row());
			indexes << index;
			selectedTracks.append(mc.canonicalUrl());
		}
		emit aboutToSendToTagEditor(indexes, selectedTracks);
	});
	/// TODO
	//connect(actionInlineTag, &QAction::triggered, this, &Playlist::editTagInline);

	connect(mediaPlaylist(), &QMediaPlaylist::loaded, this, [=] () {
		QList<QMediaContent> medias;
		for (int i = 0; i < mediaPlaylist()->mediaCount(); i++) {
			medias << mediaPlaylist()->media(i);
		}
		_playlistModel->insertMedias(-1, medias);
	});

	// No pity: marks everything as a dirty region
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](const QItemSelection & selected, const QItemSelection &) {
		this->setDirtyRegion(QRegion(this->viewport()->rect()));
		_previouslySelectedRows = selected.indexes();
		emit selectionHasChanged(selected.isEmpty());
	});

	QList<QScrollBar*> scrollBars = QList<QScrollBar*>() << horizontalScrollBar() << verticalScrollBar();
	for (QScrollBar *scrollBar : scrollBars) {
		connect(scrollBar, &QScrollBar::sliderPressed, this, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderMoved, this, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderReleased, this, [=]() { viewport()->update(); });
	}
	connect(hScrollBar, &QScrollBar::sliderMoved, this, [=]() {	horizontalHeader()->viewport()->update(); });

	this->hideColumn(COL_TRACK_DAO);
	//this->installEventFilter(this);
}

uint Playlist::generateNewHash() const
{
	if (_playlistModel->mediaPlaylist()->mediaCount() == 0) {
		return 0;
	} else {
		QString hash("");
		for (int i = 0; i < _playlistModel->mediaPlaylist()->mediaCount(); i++) {
			hash += _playlistModel->mediaPlaylist()->media(i).canonicalUrl().toString();
		}
		return qHash(hash);
	}
}

bool Playlist::isModified() const
{
	if (_hash == 0) {
		if (_playlistModel->mediaPlaylist()->isEmpty()) {
			// Closing playlist but without any tracks
			return false;
		} else {
			// Closing playlist, new playlist never saved before, with tracks
			return true;
		}
	} else {
		if (_playlistModel->mediaPlaylist()->isEmpty()) {
			// All tracks were removed
			return true;
		} else {
			// Check old and new hash
			return _hash != this->generateNewHash();
		}
	}
}

void Playlist::insertMedias(int rowIndex, const QList<QMediaContent> &medias)
{
	if (rowIndex == -1) {
		rowIndex = _playlistModel->rowCount();
	}
	if (_playlistModel->insertMedias(rowIndex, medias)) {
		this->autoResize();
	}
}

/** Insert remote medias to playlist. */
void Playlist::insertMedias(int rowIndex, const QList<TrackDAO> &tracks)
{
	if (rowIndex == -1) {
		rowIndex = _playlistModel->rowCount();
	}
	_playlistModel->insertMedias(rowIndex, tracks);
	this->autoResize();
}

QSize Playlist::minimumSizeHint() const
{
	QFontMetrics fm(SettingsPrivate::instance()->font(SettingsPrivate::FF_Playlist));
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
	if (item != nullptr) {
		for (QAction *action : _trackProperties->actions()) {
			action->setText(tr(action->text().toStdString().data()));
		}
		_trackProperties->exec(event->globalPos());
	}
}

void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
	// If the source of the drag and drop is another application, do nothing?
	if (event->source() == nullptr) {
		event->ignore();
	} else {
		event->acceptProposedAction();
	}
}

void Playlist::dragLeaveEvent(QDragLeaveEvent *event)
{
	this->setProperty("dragFromTreeview", false);
	_isDragging = false;
	QTableView::dragLeaveEvent(event);
}

void Playlist::startDrag(Qt::DropActions)
{
	qDebug() << Q_FUNC_INFO;
	_isDragging = true;
	QByteArray itemData;
	//QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	QMimeData *mimeData = new QMimeData;
	mimeData->setData("playlist/x-tableview-item", itemData);
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);

	QPixmap track(":/config/music_64");
	QColor c = QApplication::palette().highlight().color();
	QPainter p;
	p.begin(&track);
	p.setPen(c);
	p.drawRect(0, 0, 63, 63);
	c.setAlpha(128);
	p.fillRect(0, 0, 63, 63, c);
	QPen pen(Qt::white);
	pen.setWidth(1);
	p.setBrush(QApplication::palette().highlight().color());
	p.setPen(pen);
	int selectedRows = selectionModel()->selectedRows().count();
	QString s = QString::number(selectedRows);
	int l = fontMetrics().width(s);
	p.setRenderHint(QPainter::Antialiasing);
	if (l > 21) {
		p.drawRoundRect((64 - l) / 2, 21, l, 21);
		if (selectedRows > 99) {
			p.drawText(0, 21, 63, 21, Qt::AlignCenter, "99+");
		} else {
			p.drawText(0, 21, 63, 21, Qt::AlignCenter, s);
		}
	} else {
		p.drawRoundRect(21, 21, 21, 21);
		p.drawText(21, 21, 21, 21, Qt::AlignCenter, s);
	}
	p.setRenderHint(QPainter::Antialiasing, false);
	p.end();
	drag->setPixmap(track);

	// Switch from standard cursor with small plus icon or with arrow icon
	if (SettingsPrivate::instance()->copyTracksFromPlaylist()) {
		drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
	} else {
		drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::MoveAction);
	}
}

void Playlist::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
	_isDragging = true;
	this->setProperty("dragFromTreeview", event->mimeData()->hasFormat("treeview/x-treeview-item"));
	this->viewport()->update();
}

/** Redefined to be able to move tracks between playlists or internally. */
void Playlist::dropEvent(QDropEvent *event)
{
	_isDragging = false;
	QObject *source = event->source();
	int row = this->indexAt(event->pos()).row();
	if (Playlist *target = qobject_cast<Playlist*>(source)) {
		// Internal drag and drop (moving tracks)
		if (target && target == this) {
			int c = -1;
			if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
				c = _mediaPlayer->playlist()->currentIndex();
				qDebug() << Q_FUNC_INFO << "we should also change highlighted track" << c << row;
			}
			QList<QStandardItem*> rowsToHighlight = _playlistModel->internalMove(indexAt(event->pos()), selectionModel()->selectedRows());
			// Highlight rows that were just moved
			for (QStandardItem *item : rowsToHighlight) {
				for (int c = 0; c < _playlistModel->columnCount(); c++) {
					QModelIndex index = _playlistModel->index(item->row(), c);
					selectionModel()->select(index, QItemSelectionModel::Select);
				}
			}
			/*if (c >= 0) {
				//_playlistModel->mediaPlaylist()->removeMedia(0, 4);

				for (int i = 0; i < _playlistModel->mediaPlaylist()->mediaCount(); i++) {

				}
			}*/
		} else if (target && target != this) {
			// If the drop occurs at the end of the playlist, indexAt is invalid
			if (row == -1) {
				row = _playlistModel->rowCount();
			}
			QList<QMediaContent> medias;
			for (QModelIndex index : target->selectionModel()->selectedRows()) {
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
			if (!SettingsPrivate::instance()->copyTracksFromPlaylist()) {
				target->removeSelectedTracks();
			}
		}
	} else if (source == cornerWidget()) {
		qDebug() << Q_FUNC_INFO << "Drop on corner widget not yet implemented";
		event->ignore();
		return;
	} else if (source == nullptr) {
		event->ignore();
		return;
	} else {
		this->setProperty("dragFromTreeview", false);
		QByteArray byteArray = event->mimeData()->data("treeview/x-treeview-item");
		if (!byteArray.isEmpty()) {
			QList<QMediaContent> medias;
			QList<QByteArray> encodedUrls = byteArray.split('|');
			for (QByteArray encodedUrl : encodedUrls) {
				medias << QMediaContent(QUrl::fromEncoded(encodedUrl));
			}
			if (Miam::showWarning(tr("playlist"), medias.count()) == QMessageBox::Ok) {
				this->insertMedias(row, medias);
			}
		}
		event->accept();
	}
	if (QDrag *drag = findChild<QDrag*>()) {
		drag->deleteLater();
	}
}

/*bool Playlist::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ShortcutOverride) {
		qDebug() << Q_FUNC_INFO << "QEvent::ShortcutOverride";
		event->ignore();
		return false;
	} else if (event->type() == QEvent::Shortcut) {
		qDebug() << Q_FUNC_INFO << "QEvent::Shortcut";
	}
	return QTableView::eventFilter(obj, event);
}*/

/** Redefined to handle escape key when editing ratings. */
void Playlist::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		// Two level escape: if there are editors or not
		if (findChildren<StarEditor*>().isEmpty()) {
			selectionModel()->clearSelection();
			parentWidget()->parentWidget()->setFocus();
		} else {
			for (int row = 0; row < _playlistModel->rowCount(); row++) {
				closePersistentEditor(_playlistModel->index(row, COL_RATINGS));
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

/** Redifined to be able to create an editor to modify star rating. */
void Playlist::mousePressEvent(QMouseEvent *event)
{
	// For drag & drop
	if (event->button() == Qt::LeftButton) {
		_dragStartPosition = event->pos();
	}
	QModelIndex index = indexAt(event->pos());
	if (index.column() == COL_RATINGS && _previouslySelectedRows.contains(index)) {
		if (index.data(PlaylistModel::RemoteMedia).toBool() == true) {
			event->accept();
		} else {
			for (QModelIndex i : selectionModel()->selectedRows(COL_RATINGS)) {
				this->openPersistentEditor(i);
			}
		}
	} else {
		QTableView::mousePressEvent(event);
	}
}

/** Redefined to display a thin line to help user for dropping tracks. */
void Playlist::paintEvent(QPaintEvent *event)
{
	QPainter p(viewport());

	if (_playlistModel && _playlistModel->rowCount() == 0) {
		QRect vp = viewport()->rect();
		if (horizontalScrollBar()->isVisible()) {
			vp.setHeight(vp.height() - horizontalScrollBar()->rect().height());
		}

		QRect sourcePix(0, 0, 96, 96);
		sourcePix.translate(vp.width() / 2 - sourcePix.width() / 2,
							vp.height() / 2 - sourcePix.height() / 2);
		if (horizontalHeader()->isVisible()) {
			vp.setHeight(vp.height() - horizontalHeader()->rect().height());
		}
		QTextOption to;
		to.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
		to.setWrapMode(QTextOption::WordWrap);
		QRect source(0, 96, vp.width(), 48);
		// Fade empty icon playlist if there is not enough space to display it
		if (vp.height() < source.height()) {
			qreal sy = (qreal) vp.height() / (qreal) source.height();
			p.save();
			p.setOpacity(sy);
			sourcePix.translate(vp.width() / 2 - source.width() / 2,
							 vp.height() / 2 - source.height() / 2);
			p.drawPixmap(sourcePix, QPixmap(":/icons/emptyPlaylist"));
			p.restore();
		} else {

			// Highlight the drop area
			if (this->property("dragFromTreeview").toBool()) {
				p.save();
				QBrush highlight = QApplication::palette().highlight();
				QPen pen(highlight.color());
				pen.setWidth(2);
				p.setPen(pen);

				// Draw a small rectangle in the viewport
				p.drawRect(this->viewport()->rect().adjusted(1, 1, -1, -2));
				p.setBrush(highlight);
				p.setPen(Qt::NoPen);
				p.drawRect(sourcePix);

				// Then change the color of the icon with the current highlighted color
				p.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
				p.drawPixmap(sourcePix, QPixmap(":/icons/emptyPlaylist"));
				p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
				p.fillRect(rect(), QApplication::palette().base());
				p.restore();
			} else {
				p.drawPixmap(sourcePix, QPixmap(":/icons/emptyPlaylist"));
			}

		}
		source.translate(vp.width() / 2 - source.width() / 2,
						 vp.height() / 2 - source.height() / 2);

		p.drawText(source, tr("This playlist is empty.\nSelect or drop tracks from your library or any external location."), to);

		if (this->property("dragFromTreeview").toBool()) {
			p.setPen(QApplication::palette().highlight().color());
		} else {
			p.setPen(QApplication::palette().mid().color());
		}
		if (isLeftToRight()) {
			p.drawLine(viewport()->rect().topLeft(), viewport()->rect().bottomLeft());
		} else {
			p.drawLine(viewport()->rect().right() - 1, viewport()->rect().top(),
					   viewport()->rect().right() - 1, viewport()->rect().bottom() - 1);
		}

	} else {
		QTableView::paintEvent(event);
		p.save();
		p.setPen(QApplication::palette().mid().color());
		if (isLeftToRight()) {
			p.drawLine(viewport()->rect().topLeft(), viewport()->rect().bottomLeft());
		} else {
			p.drawLine(viewport()->rect().topRight(), viewport()->rect().bottomRight());
		}
		if (_isDragging) {
			QPoint cursor = viewport()->mapFromGlobal(QCursor::pos());
			QRect trackRect;

			// If one is about to drop below the last track in the playlist (in an empty area), take the last row
			QModelIndex target = indexAt(cursor);
			if (target.row() == -1) {
				trackRect = visualRect(_playlistModel->index(_playlistModel->rowCount() - 1, 0));
			} else {
				trackRect = visualRect(target);
			}
			p.setPen(QApplication::palette().highlight().color());
			if (cursor.y() >= trackRect.y() + trackRect.height() / 2) {
				p.drawLine(viewport()->rect().left(), trackRect.y() + trackRect.height(),
						   viewport()->rect().right(), trackRect.y() + trackRect.height());
			} else {
				p.drawLine(viewport()->rect().left(), trackRect.y(),
						   viewport()->rect().right(), trackRect.y());
			}
		}
		p.restore();
	}
	if (!horizontalScrollBar()->isVisible()) {
		p.setPen(QApplication::palette().mid().color());
		p.drawLine(0, viewport()->rect().bottom(),
				   viewport()->rect().right(), viewport()->rect().bottom());
	}
}

int Playlist::sizeHintForColumn(int column) const
{
	if (column == COL_RATINGS) {
		return rowHeight(COL_RATINGS) * 5;
	} else if (QStandardItem *item = _playlistModel->item(0, column)) {
		int w = fontMetrics().width(item->text());
		if (w > 0) {
			QFont f = font();
			f.setBold(true);
			f.setItalic(true);
			double ratio = QFontMetrics(f).width(item->text()) / (double) w;
			return QTableView::sizeHintForColumn(column) * qMax(1.10, ratio);
		}
	}
	// Adding ten percent should be enough for most fonts in bold + italic
	return QTableView::sizeHintForColumn(column) * 1.10;
}

void Playlist::showEvent(QShowEvent *event)
{
	resizeColumnToContents(COL_TRACK_NUMBER);
	resizeColumnToContents(COL_RATINGS);
	resizeColumnToContents(COL_YEAR);
	QTableView::showEvent(event);
}

void Playlist::wheelEvent(QWheelEvent *event)
{
	QTableView::wheelEvent(event);
	this->viewport()->update();
}

void Playlist::autoResize()
{
	if (SettingsPrivate::instance()->isPlaylistResizeColumns()) {
		this->horizontalHeader()->setStretchLastSection(false);
		this->resizeColumnsToContents();
		this->horizontalHeader()->setStretchLastSection(true);
	} else {
		this->resizeColumnToContents(COL_TRACK_NUMBER);
		this->resizeColumnToContents(COL_RATINGS);
		this->resizeColumnToContents(COL_YEAR);
	}
}

/** Move selected tracks downward. */
void Playlist::moveTracksDown()
{
	/// TODO
}

/** Move selected tracks upward. */
void Playlist::moveTracksUp()
{
	QModelIndexList indexes = this->selectionModel()->selectedRows();
	int topIndex = INT_MAX;
	for (QModelIndex idx : indexes) {
		if (idx.row() < topIndex) {
			topIndex = idx.row();
		}
	}
}

/** Remove selected tracks from the playlist. */
void Playlist::removeSelectedTracks()
{
	QModelIndexList indexes = this->selectionModel()->selectedRows();
	int indexToHighlight = INT_MAX;
	int currentPlayingIndex = _playlistModel->mediaPlaylist()->currentIndex();
	int offset = 0;
	for (QModelIndex idx : indexes) {
		if (idx.row() < indexToHighlight) {
			indexToHighlight = idx.row();
		}
		if (idx.row() < currentPlayingIndex) {
			offset++;
		}
	}

	// Remove discontiguous rows
	for (int i = indexes.size() - 1; i >= 0; i--) {
		int row = indexes.at(i).row();
		_playlistModel->removeTrack(row);
	}
	_playlistModel->blockSignals(true);
	_playlistModel->mediaPlaylist()->setCurrentIndex(currentPlayingIndex - offset);
	_playlistModel->blockSignals(false);
	if (indexToHighlight < _playlistModel->rowCount()) {
		this->selectRow(indexToHighlight);
	} else {
		// Select the last one otherwise: it still can be possible to erase all
		this->selectRow(_playlistModel->rowCount() - 1);
	}
	emit contentHasChanged();
}
