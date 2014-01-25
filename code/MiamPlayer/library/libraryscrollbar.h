#ifndef LIBRARYSCROLLBAR_H
#define LIBRARYSCROLLBAR_H

#include <QScrollBar>

#include <QMouseEvent>

class LibraryScrollBar : public QScrollBar
{
	Q_OBJECT

private:
	bool _hasNotEmittedYet;

public:
	explicit LibraryScrollBar(QWidget *parent = 0);

protected:
	void mouseMoveEvent(QMouseEvent *e);

	void mouseReleaseEvent(QMouseEvent *e);

signals:
	void displayItemDelegate(bool);
};

#endif // LIBRARYSCROLLBAR_H
