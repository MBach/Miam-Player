#ifndef PAINTABLEWIDGET_H
#define PAINTABLEWIDGET_H

#include <QApplication>
#include <QPainter>
#include <QWidget>

#include "miamcore_global.h"

/**
 * \brief	The PaintableWidget class is a small class which can react to color change.
 * \details	When one is updating colors in options, this class dynamically repaints itself by adapting the background color.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY PaintableWidget : public QWidget
{
	Q_OBJECT
private:
	bool _left, _top, _right, _bottom, _halfTop;

public:
	explicit PaintableWidget(QWidget *parent)
		: QWidget(parent)
		, _left(false)
		, _top(false)
		, _right(false)
		, _bottom(false)
		, _halfTop(true)
	{}

	inline void setFrameBorder(bool left, bool top, bool right, bool bottom)
	{
		_left = left;
		_top = top;
		_right = right;
		_bottom = bottom;
	}

	inline void setHalfTopBorder(bool b) { _halfTop = b; }

protected:
	virtual void paintEvent(QPaintEvent *) override
	{
		QPainter p(this);
		p.setPen(Qt::NoPen);
		p.setBrush(QApplication::palette().base());
		p.drawRect(this->rect());
		p.setPen(QApplication::palette().mid().color());
		if ((_left && isLeftToRight()) || (_right && !isLeftToRight())) p.drawLine(rect().topLeft(), rect().bottomLeft());
		if (_top) {
			if (_halfTop) {
				if (isLeftToRight()) {
					p.drawLine(QPoint(rect().center().x() + 1, rect().y()), rect().topRight());
				} else {
					p.drawLine(rect().topLeft(), QPoint(rect().center().x(), rect().y()));
				}
			} else {
				p.drawLine(rect().topLeft(), rect().topRight());
			}
		}
		if ((_right && isLeftToRight()) || (_left && !isLeftToRight())) p.drawLine(rect().topRight(), rect().bottomRight());
		if (_bottom) p.drawLine(rect().bottomLeft(), rect().bottomRight());
	}
};

#endif // PAINTABLEWIDGET_H
