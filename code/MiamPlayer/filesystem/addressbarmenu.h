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

	/** Used for root button. */
	bool _hasSeparator;

public:
	explicit AddressBarMenu(AddressBar *addressBar);

	void appendSubfolder(AddressBarButton *button);

	bool eventFilter(QObject *, QEvent *e);

	bool hasSeparator() const;

	void insertSeparator() const;

	void moveOrHide(const AddressBarButton *b);

	void removeSubfolder(AddressBarButton *button);

public slots:
	void show();
};

#endif // ADDRESSBARMENU_H
