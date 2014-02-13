#include "playlistmodel.h"

#include <filehelper.h>
#include <settings.h>
#include "starrating.h"

#include <QFile>
#include <QTime>
#include <QUrl>

#include <QtDebug>

PlaylistModel::PlaylistModel(QObject *parent) :
	QStandardItemModel(0, 7, parent), _mediaPlaylist(new QMediaPlaylist(this))
{}

/** Clear the content of playlist. */
void PlaylistModel::clear()
{
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

void PlaylistModel::highlightCurrentTrack()
{
	QStandardItem *it = NULL;
	const QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
	if (rowCount() > 0) {
		for (int i=0; i < rowCount(); i++) {
			for (int j = 0; j < columnCount(); j++) {
				it = item(i, j);
				QFont itemFont = font;
				itemFont.setBold(false);
				itemFont.setItalic(false);
				it->setFont(itemFont);
			}
		}
		for (int j=0; j < columnCount(); j++) {
			it = item(_mediaPlaylist->currentIndex(), j);
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

void PlaylistModel::insertMedias(int rowIndex, const QList<QMediaContent> &tracks)
{
	_mediaPlaylist->insertMedia(rowIndex, tracks);
	foreach (QMediaContent track, tracks) {
		this->insertMedia(rowIndex++, track);
	}
}

void PlaylistModel::insertMedia(int rowIndex, const QMediaContent &track)
{
	FileHelper f(track);
	if (f.isValid()) {
		qDebug() << "inserting" << track.canonicalUrl();
		QString title(f.title());

		// Then, construct a new row with correct informations
		//QStandardItem *trackItem = new QStandardItem(QString::number(f->file()->tag()->track()));
		QStandardItem *trackItem = new QStandardItem(f.trackNumber());
		trackItem->setData(track.canonicalUrl());
		QStandardItem *titleItem = new QStandardItem(title);
		QStandardItem *albumItem = new QStandardItem(f.album());
		QStandardItem *lengthItem = new QStandardItem(f.length());
		QStandardItem *artistItem = new QStandardItem(f.artist());
		QStandardItem *ratingItem = new QStandardItem();
		int rating = f.rating();
		if (rating > 0) {
			StarRating r(rating);
			ratingItem->setData(QVariant::fromValue(r), Qt::DisplayRole);
		}
		QStandardItem *yearItem = new QStandardItem(f.year());

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);

		QList<QStandardItem *> widgetItems;
		widgetItems << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem << yearItem;
		this->insertRow(rowIndex, widgetItems);
	} else {
		qDebug() << "f is invalid" << track.canonicalUrl();
	}
}

/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
QList<QStandardItem*> PlaylistModel::internalMove(QModelIndex dest, QModelIndexList selectedIndexes)
{
	// After moving rows, selection is lost. We need to keep a track on previously selected indexes
	QList<QStandardItem*> rowsToHiglight;
	QList<QMediaContent> mediasToMove;

	// Sort in reverse lexical order for correctly taking rows
	qSort(selectedIndexes.begin(), selectedIndexes.end(), qGreater<QModelIndex>());
	QList<QList<QStandardItem*> > removedRows;
	foreach (QModelIndex selectedIndex, selectedIndexes) {
		int rowNumber = selectedIndex.row();
		QList<QStandardItem*> row = this->takeRow(rowNumber);
		rowsToHiglight << row.at(0);
		removedRows.append(row);
		mediasToMove.prepend(_mediaPlaylist->media(rowNumber));
		_mediaPlaylist->removeMedia(rowNumber);
	}

	// Dest equals -1 when rows are dropped at the bottom of the playlist
	int insertPoint = (dest.isValid() || dest.row() >= 0) ? dest.row() : rowCount();
	for (int i = 0; i < removedRows.count(); i++) {
		this->insertRow(insertPoint, removedRows.at(i));
	}
	// Finally, reorder the inner QMediaPlaylist
	_mediaPlaylist->insertMedia(insertPoint, mediasToMove);

	return rowsToHiglight;
}

void PlaylistModel::insertRow(int row, const QList<QStandardItem*> &items)
{
	QFont font = Settings::getInstance()->font(Settings::PLAYLIST);
	for (int i=0; i < items.length(); i++) {
		QStandardItem *item = items.at(i);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		item->setFont(font);
	}
	QStandardItemModel::insertRow(row, items);
}

/** Redefined. */
void PlaylistModel::removeRow(int row)
{
	QStandardItemModel::removeRow(row);
	_mediaPlaylist->removeMedia(row);
}
