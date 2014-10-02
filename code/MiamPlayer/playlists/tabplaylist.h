#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QDir>
#include <QTabWidget>
#include <QMouseEvent>

#include "dialogs/closeplaylistpopup.h"
#include "playlist.h"
#include "../tracksnotfoundmessagebox.h"
#include "playlistframe.h"

#include "mediabutton.h"

#include <mediaplayer.h>

class MainWindow;

/**
 * \brief		The TabPlaylist class is used to manage mutiple playlists in the MainWindow class.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TabPlaylist : public QTabWidget
{
	Q_OBJECT

private:
	/** A custom message box for handling errors. */
	TracksNotFoundMessageBox *messageBox;

	QWeakPointer<MediaPlayer> _mediaPlayer;
	ClosePlaylistPopup *_closePlaylistPopup;
	MainWindow *_mainWindow;
	QMenu *_contextMenu;

public:
	/** Default constructor. */
	TabPlaylist(QWidget *parent = 0);

	virtual ~TabPlaylist();

	/** Get the current playlist. */
	Playlist *currentPlayList() const;

	QIcon defaultIcon(QIcon::Mode mode);

	/** Redefined to forward events to children. */
	virtual bool eventFilter(QObject *obj, QEvent *event);

	inline QWeakPointer<MediaPlayer> mediaPlayer() const { return _mediaPlayer; }

	/** Get the playlist at index. */
	Playlist *playlist(int index);

	inline QList<Playlist *> playlists() {
		QList<Playlist*> _playlists;
		for (int i = 0; i < count() - 1; i++) {
			_playlists.append(this->playlist(i));
		}
		return _playlists;
	}

	/** Setter. */
	void setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer);

	void setMainWindow(MainWindow *mainWindow);

protected:
	/** Retranslate tabs' name and all playlists in this widget. */
	virtual void changeEvent(QEvent *event);

	virtual void contextMenuEvent(QContextMenuEvent * event);

private:
	void displayEmptyArea(bool isEmpty = true);

public slots:
	/** Add a new playlist tab. */
	Playlist* addPlaylist();

	/** Add external folders (from a drag and drop) to the current playlist. */
	void addExtFolders(const QList<QDir> &folders);

	/** Insert multiple tracks chosen by one from the library or the filesystem into a playlist. */
	void insertItemsToPlaylist(int rowIndex, const QStringList &tracks);

	void moveTracksDown();

	void moveTracksUp();

	/** Action sent from the menu. */
	void removeCurrentPlaylist();

	void removeSelectedTracks();

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	void updateRowHeight();

private slots:
	/** When the user is clicking on the (+) button to add a new playlist. */
	void checkAddPlaylistButton(int i);

	void closePlaylist(int index);

	void execActionFromClosePopup(QAbstractButton *action);

signals:
	/** Forward the signal. */
	void aboutToChangeMenuLabels(int);

	void aboutToSavePlaylist(int);

	void aboutToSendToTagEditor(const QList<QUrl> &tracks);

	void selectionChanged(bool isEmpty);

	void updatePlaybackModeButton();
};

#endif // TABPLAYLIST_H
