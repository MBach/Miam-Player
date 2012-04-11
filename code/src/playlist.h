#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <phonon>
#include <QWidget>
#include <QTableWidget>

using namespace Phonon;

class Playlist : public QWidget
{
    Q_OBJECT

private:
	/** List of tracks to play. */
	QList<MediaSource> sources;

	/** Convert seconds into hh:mm:ss format. */
	QString convertTrackLength(int length);

	/** The current playing track. */
	int track;

public:
	Playlist(QWidget *parent = 0);

	QList<MediaSource> *tracks() { return &sources; }

	int activeTrack() { return track; }
	void setActiveTrack(int t) { track = t; }

	/** Clear the content of playlist. */
	void clear();

	/** Add a track to this Playlist instance. */
	QTableWidgetItem *append(MediaSource m);

	QTableWidget *table () const { return tableWidget; }

	/** Retranslate header columns. */
	void retranslateUi();

private:
	QTableWidget *tableWidget;

signals:

public slots:
	/** Change the style of the current track. Moreover, this function is reused when the user is changing fonts in the settings. */
	void highlightCurrentTrack();
};

#endif // PLAYLIST_H
