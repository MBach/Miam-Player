#include "playlist.h"

#include <QApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTime>

#include <fileref.h>
#include <tag.h>

#include "settings.h"

#include "nofocusitemdelegate.h"

Playlist::Playlist(QWidget *parent) :
	QTableWidget(parent), track(-1)
{
	// Initialize values for the Header (label and horizontal resize mode)
	QStringList labels = (QStringList() << "#" << tr("Title") << tr("Album") << tr("Length") << tr("Artist") << tr("Rating") << tr("Year"));
	const QStringList untranslatedLabels = (QStringList() << "#" << "Title" << "Album" << "Length" << "Artist" << "Rating" << "Year");
	// Test: 0 = Fixed, n>0 = real ratio for each column
	const QList<int> ratios(QList<int>() << 0 << 5 << 4 << 1 << 3 << 0 << 0);

	this->setColumnCount(labels.size());
	this->setColumnHidden(5, true);
	this->setColumnHidden(6, true);
	this->setShowGrid(false);
	Settings *settings = Settings::getInstance();
	this->setStyleSheet(settings->styleSheet(this));
	this->verticalScrollBar()->setStyleSheet(settings->styleSheet(this->verticalScrollBar()));
	this->setAlternatingRowColors(settings->colorsAlternateBG());

	// Select only one row, not cell by cell
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setItemDelegate(new NoFocusItemDelegate(this));

	/// test
	this->setAcceptDrops(true);

	verticalHeader()->setVisible(false);
	this->setHorizontalHeader(new QHeaderView(Qt::Horizontal, this));
	horizontalHeader()->setStyleSheet(settings->styleSheet(horizontalHeader()));
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	horizontalHeader()->setHighlightSections(false);
	horizontalHeader()->setMovable(true);

	this->installEventFilter(horizontalHeader());

	// Context menu on header of columns
	QList<QAction*> actionColumns;
	columns = new QMenu(this);

	for (int i = 0; i < labels.size(); i++) {
		QString label = labels.at(i);
		QTableWidgetItem *item = new QTableWidgetItem(label);

		// Stores original text to switch between translations on the fly
		// Might evolve latter, start to be "unreadable"
		item->setData(Qt::UserRole+1, untranslatedLabels.at(i));
		item->setData(Qt::UserRole+2, ratios.at(i));
		this->setHorizontalHeaderItem(i, item);

		// Match actions with columns using index of labels
		QAction *actionColumn = new QAction(label, this);
		actionColumn->setData(i);
		actionColumn->setEnabled(actionColumn->text() != tr("Title"));
		actionColumn->setCheckable(true);
		actionColumn->setChecked(!isColumnHidden(i));
		actionColumns.append(actionColumn);

		// Then populate the context menu
		columns->addAction(actionColumn);
	}

	// Load columns state
	QByteArray state = settings->value("playlistColumnsState").toByteArray();
	if (!state.isEmpty()) {
		horizontalHeader()->restoreState(state);
		for (int i = 0; i < horizontalHeader()->count(); i++) {
			bool hidden = horizontalHeader()->isSectionHidden(i);
			setColumnHidden(i, hidden);
			columns->actions().at(i)->setChecked(!hidden);
		}
	}

	// Link this playlist with the Settings instance to change fonts at runtime
	connect(settings, SIGNAL(currentFontChanged()), this, SLOT(highlightCurrentTrack()));

	// Change track
	// no need to cast parent as a TabPlaylist instance
	connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), parent, SLOT(changeTrack(QTableWidgetItem*)));

	// Hide the selected column in context menu
	connect(horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showColumnsMenu(QPoint)));
	connect(columns, SIGNAL(triggered(QAction*)), this, SLOT(toggleSelectedColumn(QAction*)));
	connect(horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(saveColumnsState(int,int,int)));
}

void Playlist::dropEvent(QDropEvent *event)
{
	qDebug() << "dropEvent";
}

#include <QDragEnterEvent>
#include <QMimeData>

void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-color")) {
		qDebug() << "dragEnterEvent";
	}
	QTableWidget::dragEnterEvent(event);
}

/** Clear the content of playlist. */
void Playlist::clear()
{
	// Iterate on the table and always remove the first item
	while (rowCount() > 0) {
		removeRow(0);
	}
	track = -1;
	sources.clear();
}

/** Convert time in seconds into "mm:ss" format. */
QString Playlist::convertTrackLength(int length)
{
	QTime time = QTime(0, 0).addSecs(length);
	// QTime is not designed to handle minutes > 60
	if (time.hour() > 0) {
		return QString::number(time.hour()*60 + time.minute()).append(":").append(time.toString("ss"));
	} else {
		return time.toString("m:ss");
	}
}

void Playlist::resizeColumns()
{
	int visibleRatio = 0;
	int resizableArea = size().width() - 4;
	if (verticalScrollBar()->isVisible()) {
		resizableArea -= verticalScrollBar()->size().width();
	}
	// Resize fixed columns first, and then compute the remaining width
	for (int c = 0; c < columnCount(); c++) {
		if (!isColumnHidden(c)) {
			int ratio = horizontalHeaderItem(c)->data(Qt::UserRole+2).toInt();
			// Fixed column
			if (ratio == 0) {
				this->resizeColumnToContents(c);
				resizableArea -= columnWidth(c) - 1;
			}
			visibleRatio += ratio;
		}
	}
	for (int c = 0; c < columnCount(); c++) {
		int ratio = horizontalHeaderItem(c)->data(Qt::UserRole+2).toInt();
		// Resizable column
		if (ratio != 0) {
			int s = resizableArea * ratio / visibleRatio ;
			if (!isColumnHidden(c)) {
				this->setColumnWidth(c, s);
			}
		}
	}
}

/** Add a track to this Playlist instance. */
void Playlist::append(const MediaSource &m)
{
	// Resolve metaDatas from TagLib
	TagLib::FileRef f(m.fileName().toLocal8Bit().data());
	if (!f.isNull()) {
		sources.append(m);
		QString title(f.tag()->title().toCString());
		if (title.isEmpty()) {
			// Filename in a MediaSource doesn't handle cross-platform QDir::separator(), so '/' is hardcoded
			title = m.fileName().split('/').last();
		}

		// Then, construct a new row with correct informations
		QList<QTableWidgetItem *> widgetItems;
		QTableWidgetItem *trackItem = new QTableWidgetItem(QString::number(f.tag()->track()));
		QTableWidgetItem *titleItem = new QTableWidgetItem(title);
		QTableWidgetItem *albumItem = new QTableWidgetItem(f.tag()->album().toCString());
		QTableWidgetItem *lengthItem = new QTableWidgetItem(this->convertTrackLength(f.audioProperties()->length()));
		QTableWidgetItem *artistItem = new QTableWidgetItem(f.tag()->artist().toCString());
		QTableWidgetItem *ratingItem = new QTableWidgetItem("***");
		QTableWidgetItem *yearItem = new QTableWidgetItem(QString::number(f.tag()->year()));

		widgetItems << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem << yearItem;

		int currentRow = rowCount();
		insertRow(currentRow);

		QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
		for (int i=0; i < widgetItems.length(); i++) {
			QTableWidgetItem *item = widgetItems.at(i);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->setFont(font);
			setItem(currentRow, i, item);
			QFontMetrics fm(font);
			setRowHeight(currentRow, fm.height());
		}

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);
	}
}

/** Toggle the selected column from the context menu. */
void Playlist::toggleSelectedColumn(QAction *action)
{
	int columnIndex = action->data().toInt();
	this->setColumnHidden(columnIndex, !isColumnHidden(columnIndex));
	this->resizeColumns();
	this->saveColumnsState();
}

/** Display a context menu with the state of all columns. */
void Playlist::showColumnsMenu(const QPoint &pos)
{
	columns->exec(mapToGlobal(pos));
}

/** Save state when one checks or moves a column. */
void Playlist::saveColumnsState(int /*logicalIndex*/, int /*oldVisualIndex*/, int /*newVisualIndex*/)
{
	// The pair "playlistColumnsState" is only used in this class, so there's no need to create specific getter and setter
	Settings::getInstance()->setValue("playlistColumnsState", horizontalHeader()->saveState());
}

/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
void Playlist::highlightCurrentTrack()
{
	QTableWidgetItem *it;
	const QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
	if (rowCount() > 0) {
		for (int i=0; i < rowCount(); i++) {
			for (int j = 0; j < columnCount(); j++) {
				it = item(i, j);
				QFont itemFont = font;
				itemFont.setBold(false);
				itemFont.setItalic(false);
				it->setFont(itemFont);
				QFontMetrics fm(itemFont);
				setRowHeight(i, fm.height());
			}
		}
		for (int j=0; j < columnCount(); j++) {
			it = item(track, j);
			// If there is actually one selected track in the playlist
			if (it != NULL) {
				QFont itemFont = font;
				itemFont.setBold(true);
				itemFont.setItalic(true);
				it->setFont(itemFont);
			}
		}
	}
}

/** Remove selected tracks from the playlist. */
void Playlist::removeSelectedTracks()
{
	int r = -1;
	QList<QTableWidgetItem *> items = selectedItems();
	for (int i = 0; i < items.size(); i++) {
		QTableWidgetItem *item = items.at(i);
		if (item) {
			r = item->row();
			sources.removeAt(r);
			removeRow(r);
		}
	}
}

/** Move the selected track upward. */
void Playlist::moveTrackUp()
{
	if (currentItem()) {
		int currentRow = currentItem()->row();
		if (currentRow > 0) {
			for (int c=0; c < columnCount(); c++) {
				QTableWidgetItem *currentItem = takeItem(currentRow, c);
				QTableWidgetItem *previousItem = takeItem(currentRow-1, c);
				setItem(currentRow, c, previousItem);
				setItem(currentRow-1, c, currentItem);
			}
			sources.swap(currentRow, currentRow-1);
			setCurrentIndex(model()->index(currentRow-1, 0));
			if (currentRow == track) {
				track--;
			}
		}
	}
}

/** Move the selected track downward. */
void Playlist::moveTrackDown()
{
	if (currentItem()) {
		int currentRow = currentItem()->row();
		if (currentRow < rowCount()-1) {
			for (int c=0; c < columnCount(); c++) {
				QTableWidgetItem *currentItem = takeItem(currentRow, c);
				QTableWidgetItem *nextItem = takeItem(currentRow+1, c);
				setItem(currentRow, c, nextItem);
				setItem(currentRow+1, c, currentItem);
			}
			sources.swap(currentRow, currentRow+1);
			this->setCurrentIndex(model()->index(currentRow+1, 0));
			if (currentRow == track) {
				track++;
			}
		}
	}
}

/** Retranslate header columns. */
void Playlist::retranslateUi()
{
	for (int i=0; i < columnCount(); i++) {
		QTableWidgetItem *headerItem = horizontalHeaderItem(i);
		const QString text = tr(headerItem->data(Qt::UserRole+1).toString().toStdString().data());
		headerItem->setText(text);
		columns->actions().at(i)->setText(text);
	}
}

bool Playlist::eventFilter(QObject *watched, QEvent *event)
{
	QHeaderView *hv = qobject_cast<QHeaderView*>(watched);
	if (hv) {
		qDebug() << "ici" << event->type();
	}
	return QTableWidget::eventFilter(watched, event);
}

void Playlist::mousePressEvent(QMouseEvent *event)
{
	return QTableWidget::mousePressEvent(event);
}


void Playlist::resizeEvent(QResizeEvent *event)
{
	this->resizeColumns();
	QTableWidget::resizeEvent(event);
}


void Playlist::showEvent(QShowEvent *event)
{
	this->resizeColumns();
	QTableWidget::showEvent(event);
}
