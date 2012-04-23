#include "playlist.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QWidgetItem>

#include <fileref.h>
#include <tag.h>

#include "settings.h"

Playlist::Playlist(QWidget *parent) :
    QWidget(parent)
{
	// Copy the layout from the first playlist, which is always available
	QLayout *newLayoutPlaylist = new QHBoxLayout(this);

	// Create a new table including track name, length and album
	tableWidget = new QTableWidget(0, 4, this);
	tableWidget->verticalHeader()->setVisible(false);

	// Select only one row, not cell by cell
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Create columns
	QTableWidgetItem *itemNumber = new QTableWidgetItem("#");
	QTableWidgetItem *itemTrack = new QTableWidgetItem(tr("Title"));
	QTableWidgetItem *itemLength = new QTableWidgetItem(tr("Length"));
	QTableWidgetItem *itemAlbum = new QTableWidgetItem(tr("Album"));

	// Stores original text to switch between translations on the fly
	itemNumber->setData(Qt::UserRole+1, "#");
	itemTrack->setData(Qt::UserRole+1, "Title");
	itemLength->setData(Qt::UserRole+1, "Length");
	itemAlbum->setData(Qt::UserRole+1, "Album");

	tableWidget->setHorizontalHeaderItem(0, itemNumber);
	tableWidget->setHorizontalHeaderItem(1, itemTrack);
	tableWidget->setHorizontalHeaderItem(2, itemLength);
	tableWidget->setHorizontalHeaderItem(3, itemAlbum);

	tableWidget->setColumnWidth(0, 30);
	tableWidget->setColumnWidth(1, 150);
	tableWidget->setColumnWidth(3, 150);

	newLayoutPlaylist->addWidget(tableWidget);

	// Link this playlist with the Settings instance to change fonts at runtime
	Settings *settings = Settings::getInstance();
	connect(settings, SIGNAL(currentFontChanged()), this, SLOT(highlightCurrentTrack()));

	// Change track
	// no need to cast parent as a TabPlaylist instance
	connect(this->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), parent, SLOT(changeTrack(QTableWidgetItem*)));
}

/** Clear the content of playlist. */
void Playlist::clear()
{
	// Iterate on the table and always remove the first item
	while (tableWidget->rowCount() > 0) {
		tableWidget->removeRow(0);
	}
	sources.clear();
}

/** Convert time in seconds into "h mm:ss" format. */
QString Playlist::convertTrackLength(int length)
{
	int h=length/3600;
	int m = length/60;
	int s = length%60;
	QString time;
	if (h > 0) {
		time = QString("%1h %2:%3");
		if (m < 10) {
			time.arg(h)
				.arg(m, (int)2, (int)10, (const QChar)'0')
				.arg(s, (int)2, (int)10, (const QChar)'0');
		} else {
			time.arg(h)
				.arg(m, (int)0, (int)10, (const QChar)'0')
				.arg(s, (int)2, (int)10, (const QChar)'0');
		}
	} else {
		if (m > 0) {
			time = QString("%1:%2")
					.arg(m, (int)0, (int)10, (const QChar)'0')
					.arg(s, (int)2, (int)10, (const QChar)'0');
		} else {
			time = QString("%1").arg(s, (int)2, (int)10, (const QChar)'0');
		}
	}
	return time;
}

/** Add a track to this Playlist instance. */
QTableWidgetItem * Playlist::append(MediaSource m)
{
	// Resolve metaDatas from TagLib
	TagLib::FileRef f(m.fileName().toLocal8Bit().data());
	if (!f.isNull()) {
		sources.append(m);
		QString title(f.tag()->title().toCString(false));
		if (title.isEmpty()) {
			title = m.fileName();
		}

		// Then, construct a new row with correct informations
		QList<QTableWidgetItem *> widgetItems;
		QTableWidgetItem *trackItem = new QTableWidgetItem(QString::number(f.tag()->track()));
		QTableWidgetItem *titleItem = new QTableWidgetItem(title);
		QTableWidgetItem *lengthItem = new QTableWidgetItem(this->convertTrackLength(f.audioProperties()->length()));
		QTableWidgetItem *albumItem = new QTableWidgetItem(f.tag()->album().toCString(false));
		widgetItems << trackItem << titleItem << lengthItem << albumItem;

		int currentRow = tableWidget->rowCount();
		tableWidget->insertRow(currentRow);

		QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
		for (int i=0; i < widgetItems.length(); i++) {
			QTableWidgetItem *item = widgetItems.at(i);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->setFont(font);
			tableWidget->setItem(currentRow, i, item);
			QFontMetrics fm(font);
			tableWidget->setRowHeight(currentRow, fm.height());
		}

		trackItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
		return trackItem;
	} else {
		return NULL;
	}
}

/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
void Playlist::highlightCurrentTrack()
{
	QTableWidgetItem *item;
	const QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
	if (tableWidget->rowCount() > 0) {
		for (int i=0; i < tableWidget->rowCount(); i++) {
			for (int j = 0; j < tableWidget->columnCount(); j++) {
				item = tableWidget->item(i, j);
				QFont itemFont = font;
				itemFont.setBold(false);
				itemFont.setItalic(false);
				item->setFont(itemFont);
				QFontMetrics fm(itemFont);
				tableWidget->setRowHeight(i, fm.height());
			}
		}
		for (int j=0; j < tableWidget->columnCount(); j++) {
			item = tableWidget->item(track, j);
			// If there is actually one selected track in the playlist
			if (item != NULL) {
				QFont itemFont = font;
				itemFont.setBold(true);
				itemFont.setItalic(true);
				item->setFont(itemFont);
			}
		}
	}
}

/** Remove the selected track from the playlist. */
void Playlist::removeSelectedTrack()
{
	int i = tableWidget->currentIndex().row();
	if (i >= 0) {
		tableWidget->removeRow(i);
		sources.removeAt(i);
	}
}

/** Move the selected track upward. */
void Playlist::moveTrackUp()
{
	int currentRow = tableWidget->currentItem()->row();
	if (currentRow > 0) {
		for (int c=0; c < tableWidget->columnCount(); c++) {
			QTableWidgetItem *currentItem = tableWidget->takeItem(currentRow, c);
			QTableWidgetItem *previousItem = tableWidget->takeItem(currentRow-1, c);
			tableWidget->setItem(currentRow, c, previousItem);
			tableWidget->setItem(currentRow-1, c, currentItem);
		}
		sources.swap(currentRow, currentRow-1);
		tableWidget->setCurrentIndex(tableWidget->model()->index(currentRow-1, 0));
		if (currentRow == track) {
			track--;
		}
	}
}

/** Move the selected track downward. */
void Playlist::moveTrackDown()
{
	int currentRow = tableWidget->currentItem()->row();
	if (currentRow < tableWidget->rowCount()-1) {
		for (int c=0; c < tableWidget->columnCount(); c++) {
			QTableWidgetItem *currentItem = tableWidget->takeItem(currentRow, c);
			QTableWidgetItem *nextItem = tableWidget->takeItem(currentRow+1, c);
			tableWidget->setItem(currentRow, c, nextItem);
			tableWidget->setItem(currentRow+1, c, currentItem);
		}
		sources.swap(currentRow, currentRow+1);
		tableWidget->setCurrentIndex(tableWidget->model()->index(currentRow+1, 0));
		if (currentRow == track) {
			track++;
		}
	}
}

/** Retranslate header columns. */
void Playlist::retranslateUi()
{
	for (int i=0; i < tableWidget->columnCount(); i++) {
		QTableWidgetItem *headerItem = tableWidget->horizontalHeaderItem(i);
		headerItem->setText(tr(headerItem->data(Qt::UserRole+1).toString().toStdString().data()));
	}
}
