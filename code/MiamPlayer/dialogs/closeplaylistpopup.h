#ifndef CLOSEPLAYLISTPOPUP_H
#define CLOSEPLAYLISTPOPUP_H

#include <QDialog>

#include "ui_closeplaylistpopup.h"

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
	int _index;

	QPushButton *_deleteButton, *_replaceButton;

public:
	explicit ClosePlaylistPopup(int index, bool currentPlaylistIsEmpty, bool playlistModified, QWidget *parent = NULL);

private slots:
	void execActionFromClosePopup(QAbstractButton *action);

signals:
	void aboutToCancel();
	void aboutToRemoveTab(int playlistTabIndex);
	void aboutToSavePlaylist(int playlistTabIndex, bool overwrite);
	void aboutToDeletePlaylist(int playlistTabIndex);
};

#endif // CLOSEPLAYLISTPOPUP_H
