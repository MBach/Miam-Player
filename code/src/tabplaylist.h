#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QDir>
#include <QTabWidget>
#include <QMouseEvent>
#include <QtMultimedia/QMediaPlayer>

#include "playlist.h"
#include "tracksnotfoundmessagebox.h"

#include "mediabutton.h"

class TabPlaylist : public QTabWidget
{
	Q_OBJECT

private:
	/// XXX: make static instance?
	QMediaPlayer *_mediaPlayer;

	/** A custom message box for handling errors. */
	TracksNotFoundMessageBox *messageBox;

public:
	/** Default constructor. */
	TabPlaylist(QWidget *parent = 0);

	/** Get the current playlist. */
	Playlist *currentPlayList() const { return qobject_cast<Playlist *>(this->currentWidget()); }

    QMediaPlayer *mediaPlayer() const { return this->_mediaPlayer; }

	/** Get the playlist at index. */
	Playlist *playlist(int index) { return qobject_cast<Playlist *>(this->widget(index)); }

	/** Retranslate tabs' name and all playlists in this widget. */
	void retranslateUi();

private:
	/** Change the current track. */
	void skip(bool forward = true);

public slots:
	/** Add a new playlist tab. */
	Playlist* addPlaylist(const QString &playlistName = QString());

	/** Add external folders (from a drag and drop) to the current playlist. */
	void addExtFolders(const QList<QDir> &folders);

	/** Add multiple tracks chosen by one from the library or the filesystem into a playlist. */
	void addItemsToPlaylist(const QList<QPersistentModelIndex> &indexes, Playlist *playlist, int row = -1);

	/** Add a single track chosen by one from the library or the filesystem into the active playlist. */
	void addItemToPlaylist(const QModelIndex &index);

	/** When the user is clicking on the (+) button to add a new playlist. */
	void checkAddPlaylistButton(int i);

	/** Action sent from the menu. */
	void removeCurrentPlaylist();

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	/** Restore playlists at startup. */
	void restorePlaylists();

	/// Media actions
	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track to the previous one. */
	void skipBackward() { this->skip(false); }

	/** Change the current track to the next one. */
	void skipForward() { this->skip(); }

signals:
	void destroyed(int);
	void created();

	/** Forward the signal. */
	void aboutToChangeMenuLabels(int);

	void sendToTagEditor(const QList<QPersistentModelIndex> &);

private slots:
	/** Save playlists before exit. */
	void savePlaylists();

	void mediaStatusChanged(QMediaPlayer::MediaStatus newMediaState);
};

#endif // TABPLAYLIST_H
