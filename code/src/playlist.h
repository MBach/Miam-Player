#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <phonon>

#include <QMenu>
#include <QTableWidget>

using namespace Phonon;

class Playlist : public QTableWidget
{
	Q_OBJECT

private:
	/** List of tracks to play. */
	QList<MediaSource> sources;

	/** The current playing track. */
	int track;

	QMenu *columns;

	QMenu *trackProperties;

	bool _selected;

public:
	Playlist(QWidget *parent = 0);

	const QList<MediaSource> & tracks() { return sources; }

	inline const int & activeTrack() const { return track; }
	inline void setActiveTrack(int t) { track = t; }

	/** Add a track to this Playlist instance. */
	void append(const MediaSource &m, int row = -1);

	/** Clear the content of playlist. */
	void clear();

	/** Retranslate header columns. */
	void retranslateUi();

protected:
	/** Redefined to display a small context menu in the view. */
	void contextMenuEvent(QContextMenuEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

private:
	/** Convert time in seconds into "mm:ss" format. */
	QString convertTrackLength(int length);

	void resizeColumns();

public slots:
	void countSelectedItems();

	/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
	void highlightCurrentTrack();

	/** Move selected tracks downward. */
	void moveTracksDown();

	/** Move selected tracks upward. */
	void moveTracksUp();

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();

private slots:
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
