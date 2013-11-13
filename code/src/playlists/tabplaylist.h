#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QDir>
#include <QTabWidget>
#include <QMouseEvent>
#include <QtMultimedia/QMediaPlayer>

#include "playlist.h"
#include "tracksnotfoundmessagebox.h"

#include "mediabutton.h"

#include <QFileSystemWatcher>

class TabPlaylist : public QTabWidget
{
	Q_OBJECT

private:
	/// XXX: make static instance?
	QMediaPlayer *_mediaPlayer;

	/** A custom message box for handling errors. */
	TracksNotFoundMessageBox *messageBox;

	QString _nextAction;

	QFileSystemWatcher *_watcher;

	/** Test: used to simulate a callback.*/
	int _tabIndex;

public:
	/** Default constructor. */
	TabPlaylist(QWidget *parent = 0);

	/** Get the current playlist. */
	Playlist *currentPlayList() const { return qobject_cast<Playlist *>(this->currentWidget()); }

    QMediaPlayer *mediaPlayer() const { return this->_mediaPlayer; }

	/** Get the playlist at index. */
	Playlist *playlist(int index) { return qobject_cast<Playlist *>(this->widget(index)); }

	QList<Playlist *> playlists() {
		QList<Playlist*> _playlists;
		for (int i = 0; i < count() - 1; i++) {
			_playlists.append(this->playlist(i));
		}
		return _playlists;
	}

protected:
	/** Retranslate tabs' name and all playlists in this widget. */
	void changeEvent(QEvent *event);

public slots:
	void restorePlaylists();

	/** Add a new playlist tab. */
	Playlist* addPlaylist();

	/** Add external folders (from a drag and drop) to the current playlist. */
	void addExtFolders(const QList<QDir> &folders);

	/** Append a single track chosen by one from the library or the filesystem into the active playlist. */
	void appendItemToPlaylist(const QString &track);

	/** Insert multiple tracks chosen by one from the library or the filesystem into a playlist. */
	void insertItemsToPlaylist(int rowIndex, const QStringList &tracks);

	/** Action sent from the menu. */
	void removeCurrentPlaylist() { removeTabFromCloseButton(this->tabBar()->currentIndex()); }

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	/// Media actions
	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track. */
	void skip(bool forward = true);

	void updateRowHeight();

private slots:
	/** When the user is clicking on the (+) button to add a new playlist. */
	void checkAddPlaylistButton(int i);

	void dispatchState(QMediaPlayer::State newState);

	/** Save playlists before exit. */
	void savePlaylists();

	void play(const QModelIndex &index);

signals:
	void destroyed(int);

	void created();

	/** Forward the signal. */
	void aboutToChangeMenuLabels(int);

	void sendToTagEditor(const QList<QPersistentModelIndex> &);

	void updatePlaybackModeButton();
};

#endif // TABPLAYLIST_H
