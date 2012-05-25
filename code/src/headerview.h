#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>

#include <QEvent>

class HeaderView : public QHeaderView
{
	Q_OBJECT
public:
	HeaderView(Qt::Orientation orientation, QWidget *parent = 0);

protected:
	bool event(QEvent *e);

	void mousePressEvent(QMouseEvent *e);

};

#endif // HEADERVIEW_H
