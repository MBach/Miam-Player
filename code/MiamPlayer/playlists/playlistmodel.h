#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMediaContent>
#include <QMediaPlaylist>
#include <QMenu>
#include <QStandardItemModel>

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

	void highlightCurrentTrack();

	void insertMedias(int rowIndex, const QList<QMediaContent> &tracks);

	void insertMedia(int rowIndex, const QMediaContent &track);

	/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
	QList<QStandardItem *> internalMove(QModelIndex dest, QModelIndexList selectedIndexes);

	/** Redefined. */
	void insertRow(int row, const QList<QStandardItem *> & items);

	inline QMediaPlaylist* mediaPlaylist() const { return _mediaPlaylist; }

	/** Redefined. */
	void removeRow(int row);

};

#endif // PLAYLISTMODEL_H
