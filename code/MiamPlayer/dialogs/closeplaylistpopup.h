#ifndef CLOSEPLAYLISTPOPUP_H
#define CLOSEPLAYLISTPOPUP_H

#include <QDialog>

#include "ui_closeplaylistpopup.h"

class ClosePlaylistPopup : public QDialog, public Ui::ClosePlaylistPopup
{
	Q_OBJECT
private:
	int _index;

public:
	explicit ClosePlaylistPopup(QWidget *parent = 0);

	inline void setTabToClose(int index) { _index = index; }

	inline const int index() const { return _index; }
};

#endif // CLOSEPLAYLISTPOPUP_H
