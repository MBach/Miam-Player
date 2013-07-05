#include "playlistmodel.h"

#include "settings.h"

#include <fileref.h>
#include <tag.h>

#include <QTime>
#include <QUrl>

#include <QtDebug>

#include "filehelper.h"

PlaylistModel::PlaylistModel(QObject *parent) :
	QStandardItemModel(parent)
{}

/** Clear the content of playlist. */
void PlaylistModel::clear()
{
	// Iterate on the table and always remove the first item
	while (rowCount() > 0) {
		removeRow(0);
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
	//TagLib::FileRef f(track.canonicalUrl().toLocalFile().toStdWString().data());
	FileHelper f(track.canonicalUrl().toLocalFile());
	if (f.file()->isValid()) {

		QString title(f.file()->tag()->title().toCString(true));

		// Then, construct a new row with correct informations
		QList<QStandardItem *> widgetItems;
		QStandardItem *trackItem = new QStandardItem(QString::number(f.file()->tag()->track()));
		QStandardItem *titleItem = new QStandardItem(title);
		QStandardItem *albumItem = new QStandardItem(f.file()->tag()->album().toCString(true));
		QStandardItem *lengthItem = new QStandardItem(QDateTime::fromTime_t(f.file()->audioProperties()->length()).toString("m:ss"));
		QStandardItem *artistItem = new QStandardItem(f.file()->tag()->artist().toCString(true));
		QStandardItem *ratingItem;
		if (f.rating() < 0) {
			ratingItem = new QStandardItem();
		} else {
			ratingItem = new QStandardItem(QString::number(f.rating()));
		}
		QStandardItem *yearItem = new QStandardItem(QString::number(f.file()->tag()->year()));

		trackItem->setData(track.canonicalUrl());

		widgetItems << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem << yearItem;
		this->insertRow(rowIndex, widgetItems);

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);
	}
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
