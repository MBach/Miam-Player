#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>

class LibraryFilterLineEdit : public QLineEdit
{
	Q_OBJECT

private:
	QToolButton *clearButton;

public:
	LibraryFilterLineEdit(QWidget *parent = 0);

protected:
	/** Keep the clear button on the right. */
	void resizeEvent(QResizeEvent *);

private slots:
	/** Redefined to add custom behaviour. */
	void clear();

	/** Show or hide the clear button. */
	void updateClearButtonStatus(const QString &text = QString());
};

#endif // LIBRARYFILTERLINEEDIT_H
