#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <QLineEdit>
#include <QPropertyAnimation>
#include <QShortcut>
#include <QTimer>

#include "miamcore_global.h"

/**
 * \brief		The LibraryFilterLineEdit class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY LibraryFilterLineEdit : public QLineEdit
{
	Q_OBJECT

private:
	QTimer *_timer;
	int _fps;
	QPropertyAnimation _fade;
	QShortcut *_shortcut;

public:
	LibraryFilterLineEdit(QWidget *parent = 0);

	QShortcut * shortcut();

protected:
	virtual void focusInEvent(QFocusEvent *e);

	virtual void focusOutEvent(QFocusEvent *e);

	virtual void paintEvent(QPaintEvent *);
};

#endif // LIBRARYFILTERLINEEDIT_H
