#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QMenu>
#include <QTableView>

#include "playlistmodel.h"

class Playlist : public QTableView
{
	Q_OBJECT

private:
	QMenu *columns;
	QMenu *trackProperties;

	/** Model to store basic fields on a track. Mostly used to create or move rows. */
	PlaylistModel *_playlistModel;

	/** Each instance of Playlist has its own QMediaPlaylist*/
	QMediaPlaylist *qMediaPlaylist;

	QModelIndex *_dropDownIndex;

	QPoint _dragStartPosition;

	Q_ENUMS(Columns)

public:
	enum Columns{TRACK_NUMBER = 0,
				 TITLE = 1,
				 ALBUM = 2,
				 ARTIST = 3,
				 LENGTH = 4,
				 RATINGS = 5,
				 YEAR = 6};

	Playlist(QWidget *parent);

	QMediaPlaylist *mediaPlaylist() { return qMediaPlaylist; }

	/** Retranslate header columns. */
	void retranslateUi();

	void init();

	void insertMedias(int rowIndex, const QList<QMediaContent> &medias);

	QSize minimumSizeHint() const;

protected:
	/** Redefined to display a small context menu in the view. */
	void contextMenuEvent(QContextMenuEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

	/** Redefined to be able to move tracks between playlists or internally. */
	void dropEvent(QDropEvent *event);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

	/** Redefined to display a thin line to help user for dropping tracks. */
	void paintEvent(QPaintEvent *e);

	//void resizeEvent(QResizeEvent *event);

	int sizeHintForColumn(int column) const;

	void showEvent(QShowEvent *event);

public slots:
	/** Move selected tracks downward. */
	void moveTracksDown();

	/** Move selected tracks upward. */
	void moveTracksUp(int i = 1);

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();

	/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
	void highlightCurrentTrack();

private slots:

	/** Display a context menu with the state of all columns. */
	void showColumnsMenu(const QPoint &);

	/** Toggle the selected column from the context menu. */
	void toggleSelectedColumn(QAction *action);
};

#endif // PLAYLIST_H
