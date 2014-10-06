#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QMediaPlaylist>
#include <QMenu>
#include <QTableView>

#include "playlistmodel.h"
#include "model/trackdao.h"

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
	enum Columns{COL_TRACK_NUMBER	= 0,
				 COL_TITLE			= 1,
				 COL_ALBUM			= 2,
				 COL_LENGTH			= 3,
				 COL_ARTIST			= 4,
				 COL_RATINGS		= 5,
				 COL_YEAR			= 6,
				 COL_ICON			= 7,
				 //COL_ID				= 8,	// Columns that should always be hidden
				 //COL_URI			= 9};
				 COL_TRACK_DAO		= 8};

	explicit Playlist(QWeakPointer<MediaPlayer> mediaPlayer, QWidget *parent = NULL);

	virtual ~Playlist();

	inline QMediaPlaylist *mediaPlaylist() { return _playlistModel->mediaPlaylist(); }

	inline uint hash() const { return _hash; }

	void insertMedias(int rowIndex, const QList<QMediaContent> &medias);

	void insertMedias(int rowIndex, const QStringList &tracks);

	/** Insert remote medias to playlist. */
	void insertMedias(int rowIndex, const QList<TrackDAO> &tracks);

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

	/** Redifined to be able to create an editor to modify star rating. */
	virtual void mousePressEvent(QMouseEvent *event);

	/** Redefined to display a thin line to help user for dropping tracks. */
	virtual void paintEvent(QPaintEvent *e);

	virtual int sizeHintForColumn(int column) const;

	virtual void showEvent(QShowEvent *event);

private:
	void autoResize();

public slots:
	/** Move selected tracks downward. */
	void moveTracksDown();

	/** Move selected tracks upward. */
	void moveTracksUp();

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();

signals:
	void aboutToSendToTagEditor(const QList<QUrl> &tracks);

	void selectionChanged(bool isEmpty);
};

#endif // PLAYLIST_H
