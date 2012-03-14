#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <QLineEdit>

class LibraryFilterLineEdit : public QLineEdit
{
	Q_OBJECT

private:
	QString defaultSearchText;

public:
	LibraryFilterLineEdit(QWidget *parent = 0);

protected:
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

signals:
	
public slots:
	
};

#endif // LIBRARYFILTERLINEEDIT_H
