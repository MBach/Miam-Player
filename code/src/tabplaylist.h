#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QTabWidget>
#include <QMouseEvent>

#include "playlist.h"

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

	MediaObject *media() { return this->mediaObject; }

	/** Convenient getter with cast. */
	Playlist *currentPlayList() const;

	/** Retranslate tabs' name and all playlists in this widget. */
	void retranslateUi();

public slots:
	/** When the user is double clicking on a track in a playlist. */
	void changeTrack(QTableWidgetItem *);

	/** Change the current track to the previous one. */
	void skipBackward();

	/** Change the current track to the next one. */
	void skipForward();
	//TODO : seekBackward, seekForward,

	void addItemFromLibraryToPlaylist(const QPersistentModelIndex &item);

	/** Action sent from the menu. */
	void removeCurrentPlaylist();

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

private slots:
	void tick(qint64 time);
	void stateChanged(Phonon::State newState, Phonon::State oldState);
	void metaStateChanged(Phonon::State newState, Phonon::State oldState);
};

#endif // TABPLAYLIST_H
