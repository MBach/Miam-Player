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
 *				to another, or in the same playlist. You can also drop external files or folder into this table to create a new playlist.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class Playlist : public QTableView
{
	Q_OBJECT

private:
	MediaPlayer *_mediaPlayer;

	/** TEST: a very basic context menu with one action: remove selected tracks. */
	QMenu *_trackProperties;

	/** Model to store basic fields on a track. Mostly used to create or move rows. */
	PlaylistModel *_playlistModel;

	QPoint _dragStartPosition;

	/** TEST: for star ratings. */
	QModelIndexList _previouslySelectedRows;

	/** Drag & drop events: when moving tracks, displays a thin line under the cursor. */
	bool _isDragging;

	uint _hash;

	uint _id;

	QString _title;

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
				 COL_TRACK_DAO		= 8};

	explicit Playlist(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

	inline MediaPlayer *mediaPlayer() const { return _mediaPlayer; }

	inline MediaPlaylist *mediaPlaylist() const { return _playlistModel->mediaPlaylist(); }

	uint generateNewHash() const;

	inline uint id() const { return _id; }
	bool isModified() const;
	inline QString title() const { return _title; }

	void insertMedias(int rowIndex, const QList<QMediaContent> &medias);

	/** Insert remote medias to playlist. */
	void insertMedias(int rowIndex, const QList<TrackDAO> &tracks);

	virtual QSize minimumSizeHint() const override;

	inline void forceDrop(QDropEvent *e) { this->dropEvent(e); }

	inline uint hash() const { return _hash; }
	inline void setHash(uint hash) { _hash = hash; }
	inline void setId(uint id) { _id = id; }
	inline void setTitle(const QString &title) { _title = title; }

protected:

	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
	virtual void dragMoveEvent(QDragMoveEvent *event) override;
	virtual void startDrag(Qt::DropActions) override;

	/** Redefined to be able to move tracks between playlists or internally. */
	virtual void dropEvent(QDropEvent *event) override;

	/** Redefined to handle escape key when editing ratings. */
	virtual void keyPressEvent(QKeyEvent *event) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;

	/** Redefined to be able to create an editor to modify star rating. */
	virtual void mousePressEvent(QMouseEvent *event) override;

	/** Redefined to display a thin line to help user for dropping tracks. */
	virtual void paintEvent(QPaintEvent *e) override;

	virtual int sizeHintForColumn(int column) const override;

	virtual void showEvent(QShowEvent *event) override;

	virtual void wheelEvent(QWheelEvent *event) override;

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

	void contentHasChanged();

	void selectionHasChanged(bool isEmpty);
};

#endif // PLAYLIST_H
