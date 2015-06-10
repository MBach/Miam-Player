#ifndef CLOSEPLAYLISTPOPUP_H
#define CLOSEPLAYLISTPOPUP_H

#include <QDialog>

#include "ui_closeplaylistpopup.h"

class Playlist;

/**
 * \brief		The ClosePlaylistPopup class is a small modal dialog
 * \details		This dialog is displayed on screen when one is about to close a playlist. 3 actions are possible for the user:
 *	- Save the current playlist in database, or overwrite it if already exists
 *  - Discard (do not save contents) and close the tab
 *  - Cancel
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ClosePlaylistPopup : public QDialog, public Ui::ClosePlaylistPopup
{
	Q_OBJECT
private:
	Playlist *_playlist;
	int _index;

	QPushButton *_deleteButton, *_replaceButton;

public:
	explicit ClosePlaylistPopup(Playlist *playlist, int index, QWidget *parent = NULL);

private slots:
	void execActionFromClosePopup(QAbstractButton *action);

signals:
	void aboutToCancel();
	void aboutToRemoveTab(int index);
	void aboutToSavePlaylist(bool overwrite);
	void aboutToDeletePlaylist(uint playlistId);
};

#endif // CLOSEPLAYLISTPOPUP_H
