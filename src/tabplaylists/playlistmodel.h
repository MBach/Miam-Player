#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMediaContent>
#include <QMediaPlaylist>
#include <QMenu>
#include <QStandardItemModel>

#include <model/trackdao.h>
#include <filehelper.h>
#include <mediaplaylist.h>
#include "miamtabplaylists_global.hpp"

/**
 * \brief		The PlaylistModel class is the underlying class for Playlist class.
 * \details		This class add tracks in a table
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
	Q_ENUMS(Origin)
private:
	/** Each instance of PlaylistModel has its own MediaPlaylist. */
	MediaPlaylist *_mediaPlaylist;

public:
	explicit PlaylistModel(QObject *parent);

	virtual ~PlaylistModel();

	enum Origin { RemoteMedia = Qt::UserRole + 1 };

	/** Clear the content of playlist. */
	void clear();

	bool insertMedias(int rowIndex, const QList<QMediaContent> &tracks);

	bool insertMedias(int rowIndex, const QList<TrackDAO> &tracks);

	/** Moves rows from various positions to a new one (discontiguous rows are grouped). */
	QList<QStandardItem *> internalMove(QModelIndex dest, QModelIndexList selectedIndexes);

	/** Redefined. XXX: warning this method is not marked as Virtual in QStandardItemModel. */
	void insertRow(int row, const QList<QStandardItem *> & items);

	inline MediaPlaylist* mediaPlaylist() const { return _mediaPlaylist; }

	void reload();

	void removeTrack(int row);

private:
	void createLine(int row, const TrackDAO &track);

	void insertMedia(int rowIndex, const FileHelper &fileHelper);
};

#endif // PLAYLISTMODEL_H
