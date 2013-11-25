#include "playlistmodel.h"

#include <filehelper.h>
#include <settings.h>
#include "starrating.h"

//#include <fileref.h>
//#include <tag.h>

#include <QFile>
#include <QTime>
#include <QUrl>

#include <QtDebug>

PlaylistModel::PlaylistModel(QObject *parent) :
	QStandardItemModel(0, 7, parent)
{}

/** Clear the content of playlist. */
void PlaylistModel::clear()
{
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

void PlaylistModel::insertMedias(int rowIndex, const QList<QMediaContent> &tracks)
{
	foreach (QMediaContent track, tracks) {
		this->insertMedia(rowIndex++, track);
	}
}

void PlaylistModel::insertMedia(int rowIndex, const QMediaContent &track)
{
	FileHelper *f = new FileHelper(track);
	if (f->isValid()) {
		QString title(f->title());

		// Then, construct a new row with correct informations
		//QStandardItem *trackItem = new QStandardItem(QString::number(f->file()->tag()->track()));
		QStandardItem *trackItem = new QStandardItem(f->trackNumber());
		trackItem->setData(track.canonicalUrl());
		QStandardItem *titleItem = new QStandardItem(title);
		QStandardItem *albumItem = new QStandardItem(f->album());
		QStandardItem *lengthItem = new QStandardItem(f->length());
		QStandardItem *artistItem = new QStandardItem(f->artist());
		QStandardItem *ratingItem = new QStandardItem();
		int rating = f->rating();
		if (rating > 0) {
			StarRating r(rating);
			ratingItem->setData(QVariant::fromValue(r), Qt::DisplayRole);
		}
		QStandardItem *yearItem = new QStandardItem(f->year());

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);

		QList<QStandardItem *> widgetItems;
		widgetItems << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem << yearItem;
		this->insertRow(rowIndex, widgetItems);
	}
	delete f;
}

/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
QList<QStandardItem*> PlaylistModel::internalMove(QModelIndex dest, QModelIndexList selectedIndexes)
{
	// After moving rows, selection is lost. We need to keep a track on previously selected indexes
	QList<QStandardItem*> rowsToHiglight;

	// Sort in reverse lexical order for correctly taking rows
	qSort(selectedIndexes.begin(), selectedIndexes.end(), qGreater<QModelIndex>());

	QList<QList<QStandardItem*> > removedRows;
	foreach (QModelIndex selectedIndex, selectedIndexes) {
		QList<QStandardItem*> row = this->takeRow(selectedIndex.row());
		rowsToHiglight << row.at(0);
		removedRows.append(row);
	}

	// Dest equals -1 when rows are dropped at the bottom of the playlist
	int insertPoint = (dest.isValid() || dest.row() >= 0) ? dest.row() : rowCount();
	for (int i = 0; i < removedRows.count(); i++) {
		this->insertRow(insertPoint, removedRows.at(i));
	}
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
