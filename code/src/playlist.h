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

public:
	Playlist(QWidget *parent = 0);

	const QList<MediaSource> & tracks() { return sources; }

	const int & activeTrack() const { return track; }
	void setActiveTrack(int t) { track = t; }

	/** Clear the content of playlist. */
	void clear();

	/** Add a track to this Playlist instance. */
	void append(const MediaSource &m);

	/** Retranslate header columns. */
	void retranslateUi();

	bool eventFilter(QObject *watched, QEvent *event);

protected:
	void mousePressEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *event);

	void showEvent(QShowEvent *event);

	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);


private:
	/** Convert time in seconds into "mm:ss" format. */
	QString convertTrackLength(int length);

	void resizeColumns();

private slots:
	/** Display a context menu with the state of all columns. */
	void showColumnsMenu(const QPoint &);

	/** Save state when one checks or moves a column. */
	void saveColumnsState(int column = 0, int oldIndex = 0, int newIndex = 0);

	/** Toggle the selected column from the context menu. */
	void toggleSelectedColumn(QAction *action);

public slots:
	/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
	void highlightCurrentTrack();

	/** Remove selected tracks from the playlist. */
	void removeSelectedTracks();

	/** Move the selected track upward. */
	void moveTrackUp();

	/** Move the selected track downward. */
	void moveTrackDown();
};

#endif // PLAYLIST_H
