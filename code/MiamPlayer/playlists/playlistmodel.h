#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMediaContent>
#include <QMediaPlaylist>
#include <QMenu>
#include <QStandardItemModel>

#include "filehelper.h"
#include "model/trackdao.h"
#include "mediaplaylist.h"

/**
 * @brief The PlaylistModel class
 */
class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
	Q_ENUMS(Origin)
private:
	/** Each instance of PlaylistModel has its own MediaPlaylist. */
	MediaPlaylist *_mediaPlaylist;

public:
	explicit PlaylistModel(QObject *parent);

	enum Origin { RemoteMedia = Qt::UserRole + 1 };

	/** Redefined to add lazy-loading. */
	//bool canFetchMore(const QModelIndex &index) const;

	/** Clear the content of playlist. */
	void clear();

	/** Redefined to add lazy-loading. */
	//void fetchMore(const QModelIndex &parent);

	bool insertMedias(int rowIndex, const QList<QMediaContent> &tracks);

	bool insertMedias(int rowIndex, const QStringList &tracks);

	bool insertMedias(int rowIndex, const QList<TrackDAO> &tracks);

	/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
	QList<QStandardItem *> internalMove(QModelIndex dest, QModelIndexList selectedIndexes);

	/** Redefined. */
	void insertRow(int row, const QList<QStandardItem *> & items);

	inline MediaPlaylist* mediaPlaylist() const { return _mediaPlaylist; }

	void removeTrack(int row);

private:
	void createLine(int row, const TrackDAO &track);

	void insertMedia(int rowIndex, const FileHelper &fileHelper);
};

#endif // PLAYLISTMODEL_H
