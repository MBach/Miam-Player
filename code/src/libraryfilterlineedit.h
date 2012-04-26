#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>

class LibraryFilterLineEdit : public QLineEdit
{
	Q_OBJECT

private:
	QString defaultSearchText;

	QToolButton *clearButton;

public:
	LibraryFilterLineEdit(QWidget *parent = 0);

protected:
	/** Reimplemented from QLineEdit::focusInEvent(). */
	void focusInEvent(QFocusEvent *);

	/** Reimplemented from QLineEdit::focusOutEvent(). */
	void focusOutEvent(QFocusEvent *);

	/** Keep the clear button on the right. */
	void resizeEvent(QResizeEvent *);

signals:
	
private slots:
	/** Redefined to add custom behaviour. */
	void clear();

	/** Show or hide the clear button. */
	void updateClearButtonStatus(const QString &text = QString());
};

#endif // LIBRARYFILTERLINEEDIT_H
