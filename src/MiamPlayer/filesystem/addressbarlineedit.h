#ifndef ADDRESSBARLINEEDIT_H
#define ADDRESSBARLINEEDIT_H

#include <QLineEdit>

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
private:
	AddressBar *_addressBar;

public:
	AddressBarLineEdit(AddressBar *parent);

protected:
	virtual void focusOutEvent(QFocusEvent *e) override;

	virtual void keyPressEvent(QKeyEvent *e) override;


};

#endif // ADDRESSBARLINEEDIT_H
