#ifndef ADDRESSBARLINEEDIT_H
#define ADDRESSBARLINEEDIT_H

#include <QLineEdit>
#include "addressbardirectorylist.h"

/// Forward declaration
class AddressBar;

/**
 * \brief		The AddressBarLineEdit class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class AddressBarLineEdit : public QLineEdit
{
	Q_OBJECT
private:
	AddressBar *_addressBar;
	AddressBarDirectoryList *_directoryList;

public:
	AddressBarLineEdit(AddressBar *parent);

protected:
	virtual void focusOutEvent(QFocusEvent *e) override;

	virtual void keyPressEvent(QKeyEvent *e) override;

	virtual void paintEvent(QPaintEvent *e) override;

signals:
	void aboutToReloadAddressBar(const QString &dirPath);
};

#endif // ADDRESSBARLINEEDIT_H
