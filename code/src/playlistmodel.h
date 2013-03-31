#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMenu>
#include <QStandardItemModel>

#include <QMediaPlaylist>

/**
 * @brief The PlaylistModel class
 */
class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
private:
	/** The current playing track. */
	int track;

	QMediaPlaylist *qMediaPlaylist;

public:
	explicit PlaylistModel(QMediaPlaylist *mediaPlaylist);

	/** Clear the content of playlist. */
	void clear();

	void move(const QModelIndexList &rows, int destChild);

	void createRows(const QList<QMediaContent> &tracks);

	void createRow(const QMediaContent &tracks);

private:
	/** Convert time in seconds into "mm:ss" format. */
	QString static convertTrackLength(int length);
};

#endif // PLAYLISTMODEL_H
