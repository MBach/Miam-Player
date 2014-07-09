#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMediaContent>
#include <QMediaPlaylist>
#include <QMenu>
#include <QStandardItemModel>

#include "filehelper.h"

/**
 * @brief The PlaylistModel class
 */
class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
private:
	/** Each instance of PlaylistModel has its own QMediaPlaylist. */
	QMediaPlaylist *_mediaPlaylist;

public:
	explicit PlaylistModel(QObject *parent);

	/** Clear the content of playlist. */
	void clear();

	void insertMedias(int rowIndex, const QList<QMediaContent> &tracks);

	/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
	QList<QStandardItem *> internalMove(QModelIndex dest, QModelIndexList selectedIndexes);

	/** Redefined. */
	void insertRow(int row, const QList<QStandardItem *> & items);

	inline QMediaPlaylist* mediaPlaylist() const { return _mediaPlaylist; }

	void removeTrack(int row);

private:
	void insertMedia(int rowIndex, const FileHelper &fileHelper);
};

#endif // PLAYLISTMODEL_H
