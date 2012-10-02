#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMenu>
#include <QStandardItemModel>

#include <phonon>

using namespace Phonon;

class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
private:
	/** The current playing track. */
	int track;

public:
	explicit PlaylistModel(QObject *parent = 0);

	inline const int & activeTrack() const { return track; }
	inline void setActiveTrack(int t) { track = t; }

	/** Add a track to this Playlist instance. */
	void append(const MediaSource &m, int row = -1);

	/** Clear the content of playlist. */
	void clear();

	void move(const QModelIndexList &rows, int destChild);

private:
	/** Convert time in seconds into "mm:ss" format. */
	QString static convertTrackLength(int length);
};

#endif // PLAYLISTMODEL_H
