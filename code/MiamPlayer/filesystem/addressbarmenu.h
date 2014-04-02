#ifndef ADDRESSBARMENU_H
#define ADDRESSBARMENU_H

#include <QListWidget>

#include "addressbarbutton.h"

class AddressBar;

class AddressBarMenu : public QListWidget
{
	Q_OBJECT
private:
	QList<AddressBarButton*> subfolders;

	AddressBar *_addressBar;

public:
	explicit AddressBarMenu(AddressBar *addressBar);

	bool eventFilter(QObject *, QEvent *e);

	void appendSubfolder(AddressBarButton *button);

	void removeSubfolder(AddressBarButton *button);

public slots:
	void show();

signals:
	void triggered(QAction *action);
};

#endif // ADDRESSBARMENU_H
