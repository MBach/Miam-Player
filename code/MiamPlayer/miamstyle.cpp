#include "miamstyle.h"

#include <QApplication>
#include <QScrollBar>
#include <QStyleOptionSlider>
#include <QPainter>

#include <QtDebug>

MiamStyle::MiamStyle(QStyle *parent) :
	QProxyStyle(parent)
{
}

/// XXX
void MiamStyle::drawScrollBar(QPainter *p, const QWidget *widget) const
{
	QStyleOptionSlider scrollbar;
	scrollbar.initFrom(widget);
	scrollbar.palette = QApplication::palette();

	QRect subLineRect = subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, widget);
	QRect addLineRect = subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, widget);
	QRect sliderRect = subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, widget);

	const QScrollBar *scoll = qobject_cast<const QScrollBar*>(widget);
	if (scoll->orientation() == Qt::Vertical) {
		subLineRect.adjust(0, 0, -1, 0);
		addLineRect.adjust(0, 0, -1, 0);
		sliderRect.adjust(0, 0, -1, 0);
	} else if (scoll->orientation() == Qt::Horizontal) {
		subLineRect.adjust(0, 0, 0, -1);
		addLineRect.adjust(0, 0, 0, -1);
		sliderRect.adjust(0, 0, 0, -1);
	}

	qDebug() << Q_FUNC_INFO << scoll->orientation();

	p->setPen(Qt::NoPen);
	p->setBrush(scrollbar.palette.window());
	p->drawRect(scoll->rect());

	p->setBrush(scrollbar.palette.base().color().darker(125));
	p->drawRect(sliderRect);

	// Frame border
	p->setPen(QApplication::palette().mid().color());
	//if (_top) p->drawLine(rect().topLeft(), rect().topRight());
	//if (_bottom) p->drawLine(rect().bottomLeft(), rect().bottomRight());
	//if (_left) p->drawLine(rect().topLeft(), rect().bottomLeft());
	//if (_right) p->drawLine(rect().topRight(), rect().bottomRight());

	// Highlight
	p->save();
	QPoint pos = scoll->mapFromGlobal(QCursor::pos());
	p->setPen(scrollbar.palette.highlight().color());

	if (!scoll->isSliderDown()) {
		p->setBrush(scrollbar.palette.highlight().color().lighter());
		if (subLineRect.contains(pos)) {
			p->drawRect(subLineRect);
		} else if (sliderRect.contains(pos)) {
			p->drawRect(sliderRect);
		} else if (addLineRect.contains(pos)) {
			p->drawRect(addLineRect);
		}
	} else {
		p->setBrush(scrollbar.palette.highlight().color());
		//if (_isDown == 0) {
			p->drawRect(subLineRect);
		//} else if (_isDown == 1) {
			p->drawRect(sliderRect);
		//} else if (_isDown == 2) {
			p->drawRect(addLineRect);
		//}
	}
	p->restore();

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
	p->save();
	p->setPen(scrollbar.palette.base().color().darker());
	p->setBrush(scrollbar.palette.base().color().darker());


	QTransform t;

	if (scoll->orientation() == Qt::Vertical) {
		t.scale((float) subLineRect.width() / 4.0, (float) subLineRect.width() / 4.0);
		QPolygonF up, down;
		up.append(t.map(upArrow[0]));
		up.append(t.map(upArrow[1]));
		up.append(t.map(upArrow[2]));
		down.append(t.map(downArrow[0]));
		down.append(t.map(downArrow[1]));
		down.append(t.map(downArrow[2]));
		p->translate(subLineRect.width() / 3.0, subLineRect.height() / 4.0);
		p->drawPolygon(up);
		p->translate(0, addLineRect.y());
		p->drawPolygon(down);
	} else if (scoll->orientation() == Qt::Horizontal) {
		t.scale((float) subLineRect.height() / 4.0, (float) subLineRect.height() / 4.0);
		QPolygonF left, right;
		left.append(t.map(leftArrow[0]));
		left.append(t.map(leftArrow[1]));
		left.append(t.map(leftArrow[2]));
		right.append(t.map(rightArrow[0]));
		right.append(t.map(rightArrow[1]));
		right.append(t.map(rightArrow[2]));
		p->translate(subLineRect.height() / 3.0, subLineRect.width() / 4.0);
		p->drawPolygon(left);
		p->translate(addLineRect.x(), 0);
		p->drawPolygon(right);
	}
	p->restore();
}

#include <QStyleOptionGroupBox>

void MiamStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *p, const QWidget *widget) const
{
	switch (control) {
	case CC_GroupBox:{
		const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
		QStyleOptionGroupBox gb2(*gb);
		gb2.textColor = QApplication::palette().windowText().color();
		QProxyStyle::drawComplexControl(control, &gb2, p, widget);
		break;
	}
	//case CC_ScrollBar:
	//	this->drawScrollBar(p, widget);
	//	break;
	default:
		QProxyStyle::drawComplexControl(control, option, p, widget);
	}
}

void MiamStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
	/*case PE_IndicatorArrowDown: {
	break;
	}*/
	case PE_IndicatorTabClose: {
		QIcon icon(":/icons/win/closeTabsHover");
		if (option->state.testFlag(State_MouseOver)) {
			painter->drawPixmap(0, 0, 16, 16, icon.pixmap(16, 16, QIcon::Normal));
		} else {
			painter->drawPixmap(0, 0, 16, 16, icon.pixmap(16, 16, QIcon::Disabled));
		}
		break;
	}
	default:
		QProxyStyle::drawPrimitive(element, option, painter, widget);
	}
}
