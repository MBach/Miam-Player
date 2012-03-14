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
	QModelIndex track;

public:
	Playlist(QWidget *parent = 0);

	QList<MediaSource> *tracks() { return &sources; }

	inline QModelIndex activeTrack() const { return track; }
	inline void setActiveTrack(const QModelIndex &track) { this->track = track; }

	/** Clear the content of playlist. */
	void clear();

	/** Get the current item in the playlist. */
	MediaSource currentTrack();

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
