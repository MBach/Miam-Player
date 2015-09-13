#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <abstractsearchdialog.h>
#include "styling/lineedit.h"
#include "dialogs/searchdialog.h"

#include <QPropertyAnimation>
#include <QShortcut>
#include <QTimer>

class MainWindow;

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

	inline SearchDialog* searchDialog() const { return _searchDialog; }

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

signals:
	void aboutToStartSearch(const QString &text);
};

#endif // LIBRARYFILTERLINEEDIT_H
