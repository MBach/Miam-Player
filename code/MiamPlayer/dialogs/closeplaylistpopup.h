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

public:
	QPushButton *deleteButton, *replaceButton;

	explicit ClosePlaylistPopup(QWidget *parent = 0);

	inline int index() { return _index; }

	inline void setTabToClose(int index) { _index = index; }

	void setDeleteMode(bool del);
	void setOverwriteMode(bool overwrite);

	void setVisible(bool visible);

//signals:
//	void saveAndOverwritePlaylist();
};

#endif // CLOSEPLAYLISTPOPUP_H
