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
public:
	explicit PlaylistModel(QObject *parent);

	/** Clear the content of playlist. */
	void clear();

	void createRows(const QList<QMediaContent> &tracks);

	void createRow(const QMediaContent &tracks);

	/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
	QList<QStandardItem *> internalMove(QModelIndex dest, QModelIndexList selectedIndexes);

	void insertRow(int row, const QList<QStandardItem *> & items);

};

#endif // PLAYLISTMODEL_H
