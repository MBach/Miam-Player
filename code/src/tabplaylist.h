#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QTabWidget>

#include <QMouseEvent>

#include "playlist.h"

using namespace Phonon;

class TabPlaylist : public QTabWidget
{
	Q_OBJECT

private:
	// Mp3 module
	MediaObject *mediaObject;
	MediaObject *metaInformationResolver;

public:
	/** Default constructor. */
	TabPlaylist(QWidget *parent = 0);

	/** Add a track from the filesystem (not the library) to the current playlist. */
	QTableWidgetItem *addItemToCurrentPlaylist(const QPersistentModelIndex &itemFromLibrary);

	MediaObject *media() const { return this->mediaObject; }

	/** Convenient getter with cast. */
	Playlist *currentPlayList() const;

	/** Retranslate tabs' name and all playlists in this widget. */
	void retranslateUi();

public slots:
	void addItemFromLibraryToPlaylist(const QPersistentModelIndex &item);

	/** When the user is double clicking on a track in a playlist. */
	void changeTrack(QTableWidgetItem *, bool autoscroll = false);

	/** Action sent from the menu. */
	void removeCurrentPlaylist();

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	void seekBackward();

	void seekForward();

	/** Change the current track to the previous one. */
	void skipBackward();

	/** Change the current track to the next one. */
	void skipForward();

signals:
	void iconStatusChanged(State);

private slots:
	void tick(qint64 time);
	void stateChanged(Phonon::State newState, Phonon::State oldState);
	void metaStateChanged(Phonon::State newState, Phonon::State oldState);
};

#endif // TABPLAYLIST_H
