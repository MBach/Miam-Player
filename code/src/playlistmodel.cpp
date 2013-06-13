#include "playlistmodel.h"

#include "settings.h"

#include <fileref.h>
#include <tag.h>

#include <QTime>
#include <QUrl>

#include <QtDebug>

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

void PlaylistModel::createRows(const QList<QMediaContent> &tracks)
{
	foreach (QMediaContent track, tracks) {
		this->createRow(track);
	}
}

void PlaylistModel::createRow(const QMediaContent &track)
{
	TagLib::FileRef f(track.canonicalUrl().toLocalFile().toStdWString().data());
	if (!f.isNull()) {

		QString title(f.tag()->title().toCString(true));

		// Then, construct a new row with correct informations
		QList<QStandardItem *> widgetItems;
		QStandardItem *trackItem = new QStandardItem(QString::number(f.tag()->track()));
		QStandardItem *titleItem = new QStandardItem(title);
		QStandardItem *albumItem = new QStandardItem(f.tag()->album().toCString(true));
		QStandardItem *lengthItem = new QStandardItem(QDateTime::fromTime_t(f.audioProperties()->length()).toString("m:ss"));
		QStandardItem *artistItem = new QStandardItem(f.tag()->artist().toCString(true));
		QStandardItem *ratingItem = new QStandardItem("***");
		QStandardItem *yearItem = new QStandardItem(QString::number(f.tag()->year()));

		trackItem->setData(track.canonicalUrl());

		widgetItems << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem << yearItem;
		this->insertRow(rowCount(), widgetItems);

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);
	}
}

void PlaylistModel::internalMove(QModelIndex dest, QModelIndexList selectedIndexes)
{
	// Sort in reverse lexical order for correctly taking rows
	qSort(selectedIndexes.begin(), selectedIndexes.end(), qGreater<QModelIndex>());

	QList<QList<QStandardItem*> > removedRows;
	foreach (QModelIndex selectedIndex, selectedIndexes) {
		removedRows.append(this->takeRow(selectedIndex.row()));
	}

	// Dest equals -1 when rows are dropped at the bottom of the playlist
	int insertPoint = dest.isValid() ? dest.row() : rowCount();
	/*int insertPoint;
	if (dest.isValid() && selectedIndexes.count() - dest.row() >= 0) {
		insertPoint = selectedIndexes.count() - dest.row();
	} else {
		insertPoint = rowCount();
	}*/
	for (int i = 0; i < removedRows.count(); i++) {
		this->insertRow(insertPoint, removedRows.at(i));
	}
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
