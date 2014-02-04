#ifndef LIBRARYSCROLLBAR_H
#define LIBRARYSCROLLBAR_H

#include <QMouseEvent>
#include <QScrollBar>

class LibraryScrollBar : public QScrollBar
{
	Q_OBJECT

private:
	bool _hasNotEmittedYet;

public:
	explicit LibraryScrollBar(QWidget *parent);

protected:
	void mouseMoveEvent(QMouseEvent *e);

	void mouseReleaseEvent(QMouseEvent *e);

signals:
	void displayItemDelegate(bool);
};

#endif // LIBRARYSCROLLBAR_H
