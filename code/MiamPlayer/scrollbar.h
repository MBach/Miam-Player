#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>

class ScrollBar : public QScrollBar
{
	Q_OBJECT
private:
	int _isDown;

	bool _top, _left, _bottom, _right;

public:
	explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = 0);

	void setFrameBorder(bool top, bool left, bool bottom, bool right);

protected:
	virtual void mousePressEvent(QMouseEvent *e);

	virtual void mouseReleaseEvent(QMouseEvent *e);

	virtual void paintEvent(QPaintEvent *e);
};

#endif // SCROLLBAR_H
