#include "scrollbar.h"

#include "settings.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

#include <QtDebug>

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget *parent) :
	QScrollBar(orientation, parent), _isDown(-1), _top(false), _left(false), _bottom(false), _right(false)
{}

void ScrollBar::setFrameBorder(bool top, bool left, bool bottom, bool right)
{
	_top = top;
	//if (isLeftToRight()) {
		_left = left;
		_right = right;
	//} else {
	//	_left = right;
	//	_right = left;
	//}
	_bottom = bottom;
}

void ScrollBar::mousePressEvent(QMouseEvent *e)
{
	QStyleOptionSlider scrollbar;
	initStyleOption(&scrollbar);
	QRect subLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, this);
	QRect addLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, this);
	QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, this);
	if (subLineRect.contains(e->pos())) {
		_isDown = 0;
	} else if (sliderRect.contains(e->pos())) {
		_isDown = 1;
	} else if (addLineRect.contains(e->pos())) {
		_isDown = 2;
	}
	QScrollBar::mousePressEvent(e);
}

void ScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	_isDown = -1;
	QScrollBar::mouseReleaseEvent(e);
	this->update();
}

void ScrollBar::paintEvent(QPaintEvent *)
{
	//QScrollBar::paintEvent(e);
	QStylePainter p(this);
	QStyleOptionSlider scrollbar;
	initStyleOption(&scrollbar);
	scrollbar.palette = QApplication::palette();

	QRect subLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, this);
	QRect addLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, this);
	QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, this);

	if (orientation() == Qt::Vertical) {
		subLineRect.adjust(0, 0, -1, 0);
		addLineRect.adjust(0, 0, -1, 0);
		sliderRect.adjust(0, 0, -1, 0);
	} else {
		subLineRect.adjust(0, 0, 0, -1);
		addLineRect.adjust(0, 0, 0, -1);
		sliderRect.adjust(0, 0, 0, -1);
	}

	p.setPen(Qt::NoPen);
	p.setBrush(scrollbar.palette.window());
	p.drawRect(this->rect());

	p.setBrush(scrollbar.palette.base().color().darker(125));
	p.drawRect(sliderRect);

	// Highlight
	p.save();
	QPoint pos = mapFromGlobal(QCursor::pos());
	p.setPen(scrollbar.palette.highlight().color());

	if (_isDown < 0) {
		p.setBrush(scrollbar.palette.highlight().color().lighter());
		if (subLineRect.contains(pos)) {
			p.drawRect(subLineRect);
		} else if (sliderRect.contains(pos)) {
			p.drawRect(sliderRect);
		} else if (addLineRect.contains(pos)) {
			p.drawRect(addLineRect);
		}
	} else {
		p.setBrush(scrollbar.palette.highlight().color());
		if (_isDown == 0) {
			p.drawRect(subLineRect);
		} else if (_isDown == 1) {
			p.drawRect(sliderRect);
		} else if (_isDown == 2) {
			p.drawRect(addLineRect);
		}
	}
	p.restore();

	// Draw sort indicator
	static const QPointF upArrow[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(2.0, 1.0)
	};
	static const QPointF downArrow[3] = {
		QPointF(0.0, 0.0),
		QPointF(2.0, 0.0),
		QPointF(1.0, 1.0)
	};
	static const QPointF leftArrow[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(1.0, 2.0)
	};
	static const QPointF rightArrow[3] = {
		QPointF(0.0, 0.0),
		QPointF(1.0, 1.0),
		QPointF(0.0, 2.0)
	};

	// Arrows
	p.save();
	if (scrollbar.palette.windowText().color().value() < 128) {
		p.setPen(scrollbar.palette.dark().color());
		p.setBrush(scrollbar.palette.dark());
	} else {
		p.setPen(scrollbar.palette.mid().color());
		p.setBrush(scrollbar.palette.mid());
	}

	QTransform t;
	float ratio = (float) subLineRect.height() / 4.0;
	t.scale(ratio, ratio);

	if (orientation() == Qt::Vertical) {
		QPolygonF up, down;
		up.append(t.map(upArrow[0]));
		up.append(t.map(upArrow[1]));
		up.append(t.map(upArrow[2]));
		down.append(t.map(downArrow[0]));
		down.append(t.map(downArrow[1]));
		down.append(t.map(downArrow[2]));
		p.translate(subLineRect.width() / 4.0, subLineRect.height() / 3.0);
		p.drawPolygon(up);
		p.translate(0, addLineRect.y());
		p.drawPolygon(down);
	} else {
		QPolygonF left, right;
		left.append(t.map(leftArrow[0]));
		left.append(t.map(leftArrow[1]));
		left.append(t.map(leftArrow[2]));
		right.append(t.map(rightArrow[0]));
		right.append(t.map(rightArrow[1]));
		right.append(t.map(rightArrow[2]));
		p.translate(subLineRect.height() / 3.0, subLineRect.width() / 4.0);
		p.drawPolygon(left);
		if (isLeftToRight()) {
			p.translate(addLineRect.x(), 0);
		} else {
			p.translate(subLineRect.x(), 0);
		}
		p.drawPolygon(right);
	}
	p.restore();

	// Frame border
	/// FIXME bottomLeft right, topLeft topRight!
	p.setPen(QApplication::palette().mid().color());
	if (_top) p.drawLine(rect().topLeft(), rect().topRight());
	if (_bottom) p.drawLine(rect().bottomLeft(), rect().bottomRight());
	if (_left) {
		if (isLeftToRight()) {
			p.drawLine(rect().topLeft(), rect().bottomLeft());
		} else {
			p.drawLine(rect().topRight(), rect().bottomRight());
		}
	}
	if (_right) {
		if (isLeftToRight()) {
			p.drawLine(rect().topRight(), rect().bottomRight());
		} else {
			p.drawLine(rect().topLeft(), rect().bottomLeft());
		}
	}
}
