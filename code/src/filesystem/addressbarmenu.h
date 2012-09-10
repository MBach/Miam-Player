#ifndef ADDRESSBARMENU_H
#define ADDRESSBARMENU_H

#include <QMenu>

#include "addressbarbutton.h"

class AddressBarMenu : public QMenu
{
	Q_OBJECT
private:
	QList<AddressBarButton*> subfolders;

public:
	explicit AddressBarMenu(QWidget *parent = 0);

	void appendSubfolder(AddressBarButton *button);

	void removeSubfolder(AddressBarButton *button);

	QAction * exec(const QPoint &p, bool showSubfolers, QAction *action = 0);

public slots:
	void clear();
};

#endif // ADDRESSBARMENU_H
