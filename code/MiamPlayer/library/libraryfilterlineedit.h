#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include "styling/lineedit.h"
#include <QPropertyAnimation>
#include <QShortcut>
#include <QTimer>

class MainWindow;
class SearchDialog;

/**
 * \brief		The LibraryFilterLineEdit class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class LibraryFilterLineEdit : public LineEdit
{
	Q_OBJECT

private:
	SearchDialog *_searchDialog;

public:
	LibraryFilterLineEdit(QWidget *parent = 0);

	QShortcut *shortcut;

	void init(MainWindow *mainWindow);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

	//virtual void focusInEvent(QFocusEvent *event) override;

	//virtual void focusOutEvent(QFocusEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

signals:
	//void focusIn();

	//void focusOut();

	void aboutToStartSearch(const QString &text);
};

#endif // LIBRARYFILTERLINEEDIT_H
