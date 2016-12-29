#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <styling/lineedit.h>
#include <abstractsearchdialog.h>
#include <searchbar.h>

#include <QPropertyAnimation>
#include <QShortcut>
#include <QTimer>

#include "miamlibrary_global.hpp"

/**
 * \brief		The LibraryFilterLineEdit class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryFilterLineEdit : public LineEdit
{
	Q_OBJECT

public:
	LibraryFilterLineEdit(QWidget *parent = nullptr);

	virtual ~LibraryFilterLineEdit();

	QShortcut *shortcut;

protected:
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // LIBRARYFILTERLINEEDIT_H
