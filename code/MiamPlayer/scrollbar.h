#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>

class ScrollBar : public QScrollBar
{
	Q_OBJECT
public:
	explicit ScrollBar(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *e);
};

#endif // SCROLLBAR_H
