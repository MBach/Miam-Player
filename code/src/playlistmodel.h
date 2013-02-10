#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QMenu>
#include <QStandardItemModel>

#include <QMediaPlaylist>

/**
 * @deprecated since Qt5
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

	inline const int & activeTrack() const { return track; }
	inline void setActiveTrack(int t) { track = t; }

	/** Add a track to this Playlist instance. */
	/// FIXME Qt5
	//void append(const MediaSource &m, int row = -1);

	/** Clear the content of playlist. */
	void clear();

	void move(const QModelIndexList &rows, int destChild);

private:
	/** Convert time in seconds into "mm:ss" format. */
	QString static convertTrackLength(int length);

public slots:
	void insertMedia(int start, int end);
};

#endif // PLAYLISTMODEL_H
