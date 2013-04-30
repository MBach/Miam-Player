#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMediaContent>
#include <QMenu>
#include <QStandardItemModel>

/**
 * @brief The PlaylistModel class
 */
class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
private:
	/** The current playing track. */
	int track;

public:
	explicit PlaylistModel(QObject *parent);

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
