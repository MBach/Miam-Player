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

	bool _selected;

	PlaylistModel *_playlistModel;

	QMediaPlaylist *qMediaPlaylist;
	QMediaPlayer *_mediaPlayer;

public:
	Playlist(QWidget *parent, QMediaPlayer *mediaplayer);

	/*MediaSource track(int i) {
		//return _playlistModel->tracks().at(i);
		QStandardItem *item = this->playlistModel()->item(i);
		return MediaSource(item->data().toString());
	}*/

	//PlaylistModel *playlistModel() { return _playlistModel; }

	QMediaPlaylist *mediaPlaylist() { return qMediaPlaylist; }

	/** Retranslate header columns. */
	void retranslateUi();

	void init();

protected:
	/** Redefined to display a small context menu in the view. */
	void contextMenuEvent(QContextMenuEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *event);

public slots:
	void countSelectedItems(const QItemSelection &, const QItemSelection &);

	void countSelectedItems();

	/** Move selected tracks downward. */
	void moveTracksDown();

	/** Move selected tracks upward. */
	void moveTracksUp(int i = 1);

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();

	/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
	void highlightCurrentTrack();

private slots:
	void changeTrack(int i);

	void resizeColumns();

	/** Display a context menu with the state of all columns. */
	void showColumnsMenu(const QPoint &);

	/** Save state when one checks or moves a column. */
	void saveColumnsState(int column = 0, int oldIndex = 0, int newIndex = 0);

	/** Toggle the selected column from the context menu. */
	void toggleSelectedColumn(QAction *action);

signals:
	void selectedTracks(int);
};

#endif // PLAYLIST_H
