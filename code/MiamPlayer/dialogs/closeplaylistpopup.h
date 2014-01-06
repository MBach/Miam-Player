#ifndef CLOSEPLAYLISTPOPUP_H
#define CLOSEPLAYLISTPOPUP_H

#include <QDialog>

#include "ui_closeplaylistpopup.h"

class ClosePlaylistPopup : public QDialog, public Ui::ClosePlaylistPopup
{
	Q_OBJECT
public:
	explicit ClosePlaylistPopup(QWidget *parent = 0);

signals:

public slots:

};

#endif // CLOSEPLAYLISTPOPUP_H
