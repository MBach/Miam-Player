#include "playlistmodel.h"

#include "model/sqldatabase.h"
#include "filehelper.h"
#include "settingsprivate.h"
#include "starrating.h"

#include <QFile>
#include <QTime>
#include <QUrl>

#include <algorithm>
#include <functional>

#include <QtDebug>

#include "playlistheaderview.h"

PlaylistModel::PlaylistModel(QObject *parent)
	: QStandardItemModel(0, PlaylistHeaderView::labels.count(), parent)
	, _mediaPlaylist(new MediaPlaylist(this))
{}

PlaylistModel::~PlaylistModel()
{}

/** Clear the content of playlist. */
void PlaylistModel::clear()
{
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

bool PlaylistModel::insertMedias(int rowIndex, const QList<QMediaContent> &tracks)
{
	int c = this->rowCount();
	SqlDatabase db;
	if (_mediaPlaylist->insertMedia(rowIndex, tracks)) {
		for (QMediaContent track : tracks) {
			if (track.canonicalUrl().isLocalFile()) {
				FileHelper f(track);
				if (f.isValid()) {
					this->insertMedia(rowIndex++, f);
				}
			} else {
				TrackDAO t = db.selectTrackByURI(track.canonicalUrl().toString());
				this->createLine(rowIndex++, t);
			}
		}
	}
	return c < this->rowCount();
}

bool PlaylistModel::insertMedias(int rowIndex, const QList<TrackDAO> &tracks)
{
	int c = this->rowCount();
	for (int i = 0; i < tracks.size(); i++) {
		TrackDAO track = tracks.at(i);
		QMediaContent mc(QUrl(track.uri()));
		if (_mediaPlaylist->insertMedia(rowIndex + i, mc)) {
			this->createLine(rowIndex + i, track);
		}
	}
	return c < this->rowCount();
}

void PlaylistModel::createLine(int row, const TrackDAO &track)
{
	QStandardItem *trackItem = new QStandardItem;
	if (!track.trackNumber().isEmpty()) {
		trackItem->setText(QString("%1").arg(track.trackNumber().toInt(), 2, 10, QChar('0')));
	}
	QStandardItem *titleItem = new QStandardItem(track.title());
	QStandardItem *albumItem = new QStandardItem(track.album());
	QStandardItem *lengthItem = new QStandardItem(track.length());
	QStandardItem *artistItem = new QStandardItem(track.artist());
	QStandardItem *ratingItem = new QStandardItem;
	StarRating r(track.rating());
	ratingItem->setData(QVariant::fromValue(r), Qt::DisplayRole);
	ratingItem->setData(true, RemoteMedia);
	ratingItem->setToolTip(tr("You cannot modify remote medias"));

	QStandardItem *yearItem = new QStandardItem(track.year());
	QStandardItem *iconItem = new QStandardItem;
	if (track.icon().isEmpty()) {
		iconItem->setText(tr("Local"));
		iconItem->setIcon(QIcon(":/icons/computer"));
		iconItem->setToolTip(tr("Local file"));
	} else {
		iconItem->setIcon(QIcon(track.icon()));
		iconItem->setToolTip(track.source());
	}

	QStandardItem *trackDAO = new QStandardItem;
	trackDAO->setData(QVariant::fromValue(track), Qt::DisplayRole);

	trackItem->setTextAlignment(Qt::AlignCenter);
	lengthItem->setTextAlignment(Qt::AlignCenter);
	ratingItem->setTextAlignment(Qt::AlignCenter);
	yearItem->setTextAlignment(Qt::AlignCenter);

	QList<QStandardItem *> items;
	items << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem \
		  << yearItem << iconItem << trackDAO;

	this->insertRow(row, items);
}

void PlaylistModel::insertMedia(int rowIndex, const FileHelper &fileHelper)
{
	QList<QStandardItem *> items;

	QStandardItem *trackItem, *titleItem, *albumItem, *lengthItem, *artistItem, *ratingItem, *yearItem;
	QStandardItem *trackDAO = new QStandardItem;
	QStandardItem *iconItem = new QStandardItem(tr("Local"));
	iconItem->setIcon(QIcon(":/icons/computer"));
	iconItem->setToolTip(tr("Local file"));
	if (FileHelper::suffixes(FileHelper::ET_Standard).contains(fileHelper.fileInfo().suffix())) {
		QString title;
		if (fileHelper.title().isEmpty()) {
			title = fileHelper.fileInfo().baseName();
		} else {
			title = fileHelper.title();
		}

		// Then, construct a new row with correct informations
		trackItem = new QStandardItem(fileHelper.trackNumber());
		titleItem = new QStandardItem(title);
		albumItem = new QStandardItem(fileHelper.album());
		lengthItem = new QStandardItem(fileHelper.length());
		artistItem = new QStandardItem(fileHelper.artist());
		ratingItem = new QStandardItem;
		int rating = fileHelper.rating();
		if (rating > 0) {
			StarRating r(rating);
			ratingItem->setData(QVariant::fromValue(r), Qt::DisplayRole);
			ratingItem->setData(false, RemoteMedia);
		}
		yearItem = new QStandardItem(fileHelper.year());
		trackDAO->setData(fileHelper.fileInfo().absoluteFilePath(), Qt::DisplayRole);

		trackItem->setTextAlignment(Qt::AlignCenter);
		lengthItem->setTextAlignment(Qt::AlignCenter);
		ratingItem->setTextAlignment(Qt::AlignCenter);
		yearItem->setTextAlignment(Qt::AlignCenter);

	} else {
		trackItem = new QStandardItem;
		titleItem = new QStandardItem(fileHelper.fileInfo().baseName());
		albumItem = new QStandardItem;
		lengthItem = new QStandardItem(QString::number(-1));
		artistItem = new QStandardItem;
		ratingItem = new QStandardItem;
		yearItem = new QStandardItem;
	}
	items << trackItem << titleItem << albumItem << lengthItem << artistItem << ratingItem \
		  << yearItem << iconItem << trackDAO;
	this->insertRow(rowIndex, items);
}

/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
QList<QStandardItem*> PlaylistModel::internalMove(QModelIndex dest, QModelIndexList selectedIndexes)
{
	// After moving rows, selection is lost. We need to keep a track on previously selected indexes
	QList<QStandardItem*> rowsToHiglight;
	QList<QMediaContent> mediasToMove;

	// Sort in reverse lexical order for correctly taking rows
	std::sort(selectedIndexes.begin(), selectedIndexes.end(), [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
	QList<QList<QStandardItem*>> removedRows;
	_mediaPlaylist->blockSignals(true);
	int currentPlayingTrack = _mediaPlaylist->currentIndex();
	for (QModelIndex selectedIndex : selectedIndexes) {
		int rowNumber = selectedIndex.row();
		QList<QStandardItem*> row = this->takeRow(rowNumber);
		rowsToHiglight << row.at(0);
		removedRows.append(row);
		mediasToMove.prepend(_mediaPlaylist->media(rowNumber));
		_mediaPlaylist->removeMedia(rowNumber);
		currentPlayingTrack--;
	}

	// Dest equals -1 when rows are dropped at the bottom of the playlist
	int insertPoint = (dest.isValid() || dest.row() >= 0) ? dest.row() : rowCount();
	if (insertPoint > rowCount()) {
		insertPoint = rowCount();
	}
	for (int i = 0; i < removedRows.count(); i++) {
		this->insertRow(insertPoint, removedRows.at(i));
	}

	// Finally, reorder the inner QMediaPlaylist
	_mediaPlaylist->insertMedia(insertPoint, mediasToMove);
	currentPlayingTrack += mediasToMove.size();
    _mediaPlaylist->blockSignals(false);
    return rowsToHiglight;
}

void PlaylistModel::insertRow(int row, const QList<QStandardItem*> &items)
{
	QFont font = SettingsPrivate::instance()->font(SettingsPrivate::FF_Playlist);
	for (int i = 0; i < items.length(); i++) {
		QStandardItem *item = items.at(i);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		item->setFont(font);
	}
	QStandardItemModel::insertRow(row, items);
}

void PlaylistModel::reload()
{
	for (int row = 0; row < rowCount(); row++) {
		QStandardItem *currentTrackItem = item(row, Playlist::COL_TRACK_DAO);
		FileHelper fileHelper(currentTrackItem->data(Qt::DisplayRole).toString());

		QString title;
		if (fileHelper.title().isEmpty()) {
			title = fileHelper.fileInfo().baseName();
		} else {
			title = fileHelper.title();
		}

		// Then, construct a new row with correct informations
		item(row, Playlist::COL_TRACK_NUMBER)->setData(fileHelper.trackNumber(), Qt::DisplayRole);
		item(row, Playlist::COL_TITLE)->setData(title, Qt::DisplayRole);
		item(row, Playlist::COL_ALBUM)->setData(fileHelper.album(), Qt::DisplayRole);
		item(row, Playlist::COL_LENGTH)->setData(fileHelper.length(), Qt::DisplayRole);
		item(row, Playlist::COL_ARTIST)->setData(fileHelper.artist(), Qt::DisplayRole);
		int rating = fileHelper.rating();
		if (rating > 0) {
			StarRating r(rating);
			item(row, Playlist::COL_RATINGS)->setData(QVariant::fromValue(r), Qt::DisplayRole);
			item(row, Playlist::COL_RATINGS)->setData(false, RemoteMedia);
		}
		item(row, Playlist::COL_YEAR)->setData(fileHelper.year(), Qt::DisplayRole);
	}
}

void PlaylistModel::removeTrack(int row)
{
	QStandardItemModel::removeRow(row);
	_mediaPlaylist->removeMedia(row);
	if (_mediaPlaylist->playbackMode() == QMediaPlaylist::Random) {
		_mediaPlaylist->shuffle(-1);
	}
}
