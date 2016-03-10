#include "scrollbar.h"

#include "settings.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

#include <QtDebug>

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget *parent)
	: QScrollBar(orientation, parent)
	, _isDown(-1)
	, _top(false)
	, _left(false)
	, _bottom(false)
	, _right(false)
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

	QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, this);
#if defined(Q_OS_OSX)
	QPoint p = mapFromGlobal(QCursor::pos());
	if (sliderRect.contains(p)) {
		_isDown = 1;
	}
#else
	QRect subLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, this);
	QRect addLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, this);
	QPoint p = mapFromGlobal(QCursor::pos());
	if (subLineRect.contains(p)) {
		_isDown = 0;
	} else if (sliderRect.contains(p)) {
		_isDown = 1;
	} else if (addLineRect.contains(p)) {
		_isDown = 2;
	}
#endif

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
#if defined(Q_OS_OSX)
	QStylePainter p(this);
	QStyleOptionSlider scrollbar;
	initStyleOption(&scrollbar);
	scrollbar.palette = QApplication::palette();

	QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, this);

	if (orientation() == Qt::Vertical) {
		sliderRect.adjust(sliderRect.width() * 0.25, 0, -sliderRect.width() * 0.25, 0);
	} else {
		sliderRect.adjust(0, sliderRect.height() * 0.25, 0, -sliderRect.height() * 0.25);
	}

	p.setPen(Qt::NoPen);
	p.setBrush(scrollbar.palette.window());
	p.drawRect(this->rect());

	p.setBrush(scrollbar.palette.base().color().darker(125));

	p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath painterPath;
	if (orientation() == Qt::Vertical) {

		QRect sliderTopRect(sliderRect.x(), sliderRect.y(), sliderRect.width(), sliderRect.width());
		QRect sliderBottomRect(sliderRect.x(), sliderRect.y() + sliderRect.height() - sliderRect.width(), sliderRect.width(), sliderRect.width());
		sliderRect.adjust(0, sliderRect.width(), 0, sliderRect.width());

		// 4---3---2    Top curve is painted with 2 calls to cubicTo, starting in 1     ^       ^
		// |       |    First cubic call is with points p1, p2 and p3                   |       |
		// 5---+---1    Second is with points p3, p4 and p5                             6---+--10
		// |       |                                                                    |       |
		// \/      \/                                                                   7---8---9
		painterPath.moveTo(sliderRect.x() + sliderRect.width(), sliderRect.y());
		painterPath.cubicTo(sliderTopRect.x() + sliderTopRect.width(), sliderRect.y(),
							sliderTopRect.x() + sliderTopRect.width(), sliderTopRect.y() + sliderTopRect.height() / 2.0f,
							sliderTopRect.x() + sliderTopRect.width() / 2.0f, sliderTopRect.y() + sliderTopRect.height() / 2.0f);
		painterPath.cubicTo(sliderTopRect.x() + sliderTopRect.width() / 2.0f, sliderTopRect.y() + sliderTopRect.height() / 2.0f,
							sliderTopRect.x(), sliderTopRect.y() + sliderTopRect.height() / 2.0f,
							sliderTopRect.x(), sliderTopRect.y() + sliderTopRect.height());

		painterPath.lineTo(sliderBottomRect.topLeft());
		painterPath.cubicTo(sliderBottomRect.x(), sliderBottomRect.y(),
							sliderBottomRect.x(), sliderBottomRect.y() + sliderBottomRect.height() / 2.0f,
							sliderBottomRect.x() + sliderBottomRect.width() / 2.0f, sliderBottomRect.y() + sliderBottomRect.height() / 2.0f);
		painterPath.cubicTo(sliderBottomRect.x() + sliderBottomRect.width() / 2.0f, sliderBottomRect.y() + sliderBottomRect.height() / 2.0f,
							sliderBottomRect.x() + sliderBottomRect.width(), sliderBottomRect.y() + sliderBottomRect.height() / 2.0f,
							sliderBottomRect.x() + sliderBottomRect.width(), sliderBottomRect.y());

		painterPath.lineTo(sliderRect.x() + sliderRect.width(), sliderRect.y());

	} else {
		QRect sliderLeftRect(sliderRect.x(), sliderRect.y(), sliderRect.height(), sliderRect.height());
		QRect sliderRightRect(sliderRect.x() + sliderRect.width() - sliderRect.height(), sliderRect.y(), sliderRect.height(), sliderRect.height());
		sliderRect.adjust(sliderRect.height(), 0, sliderRect.height(), 0);

		painterPath.moveTo(sliderRect.x(), sliderRect.y());
		painterPath.cubicTo(sliderRect.x(), sliderRect.y(),
					   sliderLeftRect.x() + sliderLeftRect.width() / 2.0f, sliderLeftRect.y(),
					   sliderLeftRect.x() + sliderLeftRect.width() / 2.0f, sliderLeftRect.y() + sliderLeftRect.height() / 2.0f);
		painterPath.cubicTo(sliderLeftRect.x() + sliderLeftRect.width() / 2.0f, sliderLeftRect.y() + sliderLeftRect.height() / 2.0f,
					   sliderLeftRect.x() + sliderLeftRect.width() / 2.0f, sliderLeftRect.y() + sliderLeftRect.height(),
					   sliderLeftRect.x() + sliderLeftRect.width(), sliderLeftRect.y() + sliderLeftRect.height());

		painterPath.lineTo(sliderRightRect.x(), sliderRightRect.y() + sliderRightRect.height());
		painterPath.cubicTo(sliderRightRect.x(), sliderRightRect.y() + sliderRightRect.height(),
						sliderRightRect.x() + sliderRightRect.width() / 2.0f, sliderRightRect.y() + sliderRightRect.height(),
						sliderRightRect.x() + sliderRightRect.width() / 2.0f, sliderRightRect.y() + sliderRightRect.height() / 2.0f);
		painterPath.cubicTo(sliderRightRect.x() + sliderRightRect.width() / 2.0f, sliderRightRect.y() + sliderRightRect.height() / 2.0f,
						sliderRightRect.x() + sliderRightRect.width() / 2.0f, sliderRightRect.y(),
						sliderRightRect.x(), sliderRightRect.y());

		painterPath.lineTo(sliderRect.x(), sliderRect.y());
	}

	// Highlight
	p.save();
	if (_isDown == 1) {
		p.setPen(scrollbar.palette.highlight().color());
		p.setBrush(scrollbar.palette.highlight().color().lighter());
	}
	p.drawPath(painterPath);
	p.restore();
	p.setRenderHint(QPainter::Antialiasing, false);

	// Frame border
	p.setPen(QApplication::palette().mid().color());
	if (_top) {
		p.drawLine(rect().x(), rect().y(), rect().x() + rect().width(), rect().y());
	}
	if (_bottom) {
		p.drawLine(rect().x(), rect().y() + rect().height(), rect().x() + rect().width(), rect().y() + rect().height());
	}
	if (_left) {
		if (isLeftToRight()) {
			p.drawLine(rect().x(), rect().y(), rect().x(), rect().y() + rect().height());
		} else {
			p.drawLine(rect().x() + rect().width(), rect().y(), rect().x() + rect().width(), rect().y() + rect().height());
		}
	}
	if (_right) {
		if (isLeftToRight()) {
			p.drawLine(rect().x() + rect().width() - extra, rect().y(), rect().x() + rect().width() - extra, rect().y() + rect().height());
		} else {
			p.drawLine(rect().x(), rect().y(), rect().x(), rect().y() + rect().height());
		}
	}
#else
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
		sliderRect.adjust(0, 0, -1, -1);
	} else {
		subLineRect.adjust(0, 0, 0, -1);
		addLineRect.adjust(0, 0, 0, -1);
		sliderRect.adjust(0, 0, -1, -1);
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
	p.setPen(QApplication::palette().mid().color());
	if (_top) {
		p.drawLine(rect().x(), rect().y(), rect().x() + rect().width(), rect().y());
	}
	if (_bottom) {
		p.drawLine(rect().x(), rect().y() + rect().height() - extra, rect().x() + rect().width(), rect().y() + rect().height() - extra);
	}
	if (_left) {
		if (isLeftToRight()) {
			p.drawLine(rect().x(), rect().y(), rect().x(), rect().y() + rect().height());
		} else {
			p.drawLine(rect().x() + rect().width(), rect().y(), rect().x() + rect().width(), rect().y() + rect().height());
		}
	}
	if (_right) {
		if (isLeftToRight()) {
			p.drawLine(rect().x() + rect().width() - extra, rect().y(), rect().x() + rect().width() - extra, rect().y() + rect().height());
		} else {
			p.drawLine(rect().x(), rect().y(), rect().x(), rect().y() + rect().height());
		}
	}
#endif
}
