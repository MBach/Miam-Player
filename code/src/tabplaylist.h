#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QTabWidget>

#include <QMouseEvent>

#include "playlist.h"
#include "tracksnotfoundmessagebox.h"

using namespace Phonon;

class TabPlaylist : public QTabWidget
{
	Q_OBJECT

private:
	// Mp3 module
	MediaObject *mediaObject;
	MediaObject *metaInformationResolver;

	/** A custom message box for handling errors. */
	TracksNotFoundMessageBox *messageBox;

public:
	/** Default constructor. */
	TabPlaylist(QWidget *parent = 0);

	/** Add a track from the filesystem (not the library) to the current playlist. */
	void addItemToCurrentPlaylist(const QPersistentModelIndex &itemFromLibrary);

	MediaObject *media() const { return this->mediaObject; }

	/** Convenient getter with cast. */
	Playlist *currentPlayList() const;

	/** Retranslate tabs' name and all playlists in this widget. */
	void retranslateUi();

public slots:
	/** Add a new playlist tab. */
	Playlist* addPlaylist();

	/** Add tracks chosen by one from the library into the active playlist. */
	void addItemFromLibraryToPlaylist(const QPersistentModelIndex &item);

	/** When the user is double clicking on a track in a playlist. */
	void changeTrack(QTableWidgetItem *, bool autoscroll = false);

	/** When the user is clicking on the (+) button to add a new playlist. */
	void checkAddPlaylistButton(int i);

	/** Action sent from the menu. */
	void removeCurrentPlaylist();

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	/** Restore playlists at startup. */
	void restorePlaylists();

	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track to the previous one. */
	void skipBackward();

	/** Change the current track to the next one. */
	void skipForward();

signals:
	void iconStatusChanged(State);

private slots:
	/** Save playlists before exit. */
	void savePlaylists();

	void tick(qint64 time);
	void stateChanged(Phonon::State newState, Phonon::State oldState);
	void metaStateChanged(Phonon::State newState, Phonon::State oldState);
};

#endif // TABPLAYLIST_H
