#ifndef TABPLAYLIST_H
#define TABPLAYLIST_H

#include <QDir>
#include <QTabWidget>
#include <QMouseEvent>

#include <model/playlistdao.h>
#include <mediabutton.h>
#include <mediaplayer.h>
#include "../tracksnotfoundmessagebox.h"
#include "playlist.h"
#include "playlistmanager.h"
#include "playlistframe.h"

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
	PlaylistManager *_playlistManager;

	/** A custom message box for handling errors. */
	TracksNotFoundMessageBox *messageBox;

	MainWindow *_mainWindow;
	QMenu *_contextMenu;

public:
	/** Default constructor. */
	explicit TabPlaylist(QWidget *parent = 0);

	virtual ~TabPlaylist();

	/** Get the current playlist. */
	Playlist *currentPlayList() const;

	QIcon defaultIcon(QIcon::Mode mode);

	/** Redefined to forward events to children. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	void init();

	/** Load a playlist saved in database. */
	void loadPlaylist(uint playlistId);

	/** Get the playlist at index. */
	Playlist *playlist(int index);

	inline QList<Playlist *> playlists() {
		QList<Playlist*> _playlists;
		for (int i = 0; i < count(); i++) {
			_playlists.append(this->playlist(i));
		}
		return _playlists;
	}

	void setMainWindow(MainWindow *mainWindow);

protected:
	/** Retranslate tabs' name and all playlists in this widget. */
	virtual void changeEvent(QEvent *event);

	virtual void contextMenuEvent(QContextMenuEvent * event);

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

	void removeTabs(const QList<PlaylistDAO> &playlists);

	/** Remove a playlist when clicking on a close button in the corner. */
	void removeTabFromCloseButton(int index);

	void updateRowHeight();

	int closePlaylist(int index, bool aboutToQuit = false);

	void sendTabs();

signals:
	/** Forward the signal. */
	void aboutToChangeMenuLabels(int);

	//void aboutToDeletePlaylist(int playlistTabIndex);

	void aboutToSavePlaylist(int playlistTabIndex, bool overwrite = false, bool exit = false);

	void aboutToSendToTagEditor(const QList<QUrl> &tracks);

	void selectionChanged(bool isEmpty);

	void updatePlaybackModeButton();

	void tabs(const QList<Playlist*> &unsavedPlaylists);
};

#endif // TABPLAYLIST_H
