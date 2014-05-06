#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QMediaPlaylist>
#include <QMenu>
#include <QTableView>

#include "playlistmodel.h"

#include <mediaplayer.h>

/**
 * \brief		The Playlist class is used to display tracks in the MainWindow class.
 * \details		The QTableView uses a small custom model to manage tracks: the PlaylistModel class. Tracks can be moved from one playlist
 *		to another, or in the same playlist. You can also drop external files or folder into this table to create a new playlist.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class Playlist : public QTableView
{
	Q_OBJECT

private:
	/** TEST: a very basic context menu with one action: remove selected tracks. */
	QMenu *_trackProperties;

	/** Model to store basic fields on a track. Mostly used to create or move rows. */
	PlaylistModel *_playlistModel;

	/** Reference to the unique instance of MediaPlayer class in the application. */
	QWeakPointer<MediaPlayer> _mediaPlayer;

	/** TEST: for drag & drop events: when moving tracks, displays a thin line under the cursor. */
	QModelIndex *_dropDownIndex;

	/** TEST. */
	QPoint _dragStartPosition;

	/** TEST: for star ratings. */
	QModelIndexList _previouslySelectedRows;

	uint _hash;

	Q_ENUMS(Columns)

public:
	enum Columns{TRACK_NUMBER = 0,
				 TITLE = 1,
				 ALBUM = 2,
				 ARTIST = 3,
				 LENGTH = 4,
				 RATINGS = 5,
				 YEAR = 6};

	explicit Playlist(QWeakPointer<MediaPlayer> mediaPlayer, QWidget *parent = NULL);

	inline QMediaPlaylist *mediaPlaylist() { return _playlistModel->mediaPlaylist(); }

	inline uint hash() const { return _hash; }

	void insertMedias(int rowIndex, const QList<QMediaContent> &medias);

	void insertMedias(int rowIndex, const QStringList &tracks);

	QSize minimumSizeHint() const;

	inline void forceDrop(QDropEvent *e) { this->dropEvent(e); }

	inline void setHash(uint hash) { _hash = hash; }

	inline QWeakPointer<MediaPlayer> mediaPlayer() { return _mediaPlayer; }

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);

	/** Redefined to be able to move tracks between playlists or internally. */
	virtual void dropEvent(QDropEvent *event);

	/** Redefined to handle escape key when editing ratings. */
	virtual void keyPressEvent(QKeyEvent *event);

	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);

	/** Redefined to display a thin line to help user for dropping tracks. */
	virtual void paintEvent(QPaintEvent *e);

	virtual int sizeHintForColumn(int column) const;

	virtual void showEvent(QShowEvent *event);

public slots:
	/** Move selected tracks downward. */
	void moveTracksDown();

	/** Move selected tracks upward. */
	void moveTracksUp();

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();
};

#endif // PLAYLIST_H
