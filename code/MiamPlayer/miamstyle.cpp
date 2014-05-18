#include "miamstyle.h"

#include <QApplication>
#include <QScrollBar>
#include <QStyleOptionGroupBox>
#include <QStyleOptionSlider>
#include <QPainter>

#include <QtDebug>

MiamStyle::MiamStyle(QStyle *parent) :
	QProxyStyle(parent)
{
}

/// XXX
void MiamStyle::drawScrollBar(QPainter *painter, const QWidget *widget) const
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

	painter->setPen(Qt::NoPen);
	painter->setBrush(scrollbar.palette.window());
	painter->drawRect(scoll->rect());

	painter->setBrush(scrollbar.palette.base().color().darker(125));
	painter->drawRect(sliderRect);

	// Frame border
	painter->setPen(QApplication::palette().mid().color());
	//if (_top) painter->drawLine(rect().topLeft(), rect().topRight());
	//if (_bottom) painter->drawLine(rect().bottomLeft(), rect().bottomRight());
	//if (_left) painter->drawLine(rect().topLeft(), rect().bottomLeft());
	//if (_right) painter->drawLine(rect().topRight(), rect().bottomRight());

	// Highlight
	painter->save();
	QPoint pos = scoll->mapFromGlobal(QCursor::pos());
	painter->setPen(scrollbar.palette.highlight().color());

	if (!scoll->isSliderDown()) {
		painter->setBrush(scrollbar.palette.highlight().color().lighter());
		if (subLineRect.contains(pos)) {
			painter->drawRect(subLineRect);
		} else if (sliderRect.contains(pos)) {
			painter->drawRect(sliderRect);
		} else if (addLineRect.contains(pos)) {
			painter->drawRect(addLineRect);
		}
	} else {
		painter->setBrush(scrollbar.palette.highlight().color());
		//if (_isDown == 0) {
			painter->drawRect(subLineRect);
		//} else if (_isDown == 1) {
			painter->drawRect(sliderRect);
		//} else if (_isDown == 2) {
			painter->drawRect(addLineRect);
		//}
	}
	painter->restore();

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
	painter->save();
	painter->setPen(scrollbar.palette.base().color().darker());
	painter->setBrush(scrollbar.palette.base().color().darker());


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
		painter->translate(subLineRect.width() / 3.0, subLineRect.height() / 4.0);
		painter->drawPolygon(up);
		painter->translate(0, addLineRect.y());
		painter->drawPolygon(down);
	} else if (scoll->orientation() == Qt::Horizontal) {
		t.scale((float) subLineRect.height() / 4.0, (float) subLineRect.height() / 4.0);
		QPolygonF left, right;
		left.append(t.map(leftArrow[0]));
		left.append(t.map(leftArrow[1]));
		left.append(t.map(leftArrow[2]));
		right.append(t.map(rightArrow[0]));
		right.append(t.map(rightArrow[1]));
		right.append(t.map(rightArrow[2]));
		painter->translate(subLineRect.height() / 3.0, subLineRect.width() / 4.0);
		painter->drawPolygon(left);
		painter->translate(addLineRect.x(), 0);
		painter->drawPolygon(right);
	}
	painter->restore();
}

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

#include "settings.h"

void MiamStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *opt, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
	/*case PE_IndicatorArrowDown: {
	break;
	}*/
	case PE_IndicatorTabClose: {
		if (opt->state.testFlag(State_MouseOver)) {
			painter->drawPixmap(0, 0, 16, 16, QPixmap(":/icons/win/closeTabsHover"));
		} else {
			painter->drawPixmap(0, 0, 16, 16, QPixmap(":/icons/closeTabs"));
		}
		break;
	}
	case PE_IndicatorBranch: {

		// Draw sort indicator
		static const QPointF downArrow[3] = { QPointF(0.0, 0.0), QPointF(2.0, 0.0), QPointF(1.0, 1.0) };
		static const QPointF leftArrow[3] = { QPointF(0.0, 1.0), QPointF(1.0, 0.0), QPointF(1.0, 2.0) };
		static const QPointF rightArrow[3] = { QPointF(0.0, 0.0), QPointF(1.0, 1.0), QPointF(0.0, 2.0) };

		QTransform t;
		float ratio = opt->rect.width() / 4.0;
		//qDebug() << "ratio" << ratio << opt->rect << opt->fontMetrics.height();
		t.scale(ratio, ratio);

		QPolygonF arrow;
		if (QGuiApplication::isLeftToRight()) {
			QPolygonF right;
			right.append(t.map(rightArrow[0]));
			right.append(t.map(rightArrow[1]));
			right.append(t.map(rightArrow[2]));
			arrow = right;
		} else {
			QPolygonF left;
			left.append(t.map(leftArrow[0]));
			left.append(t.map(leftArrow[1]));
			left.append(t.map(leftArrow[2]));
			arrow = left;
		}

		if (opt->state.testFlag(State_Children)) {
			painter->save();
			if (opt->state.testFlag(State_MouseOver)) {
				painter->setPen(opt->palette.highlight().color());
				painter->setBrush(opt->palette.highlight().color().lighter());
			} else {
				painter->setPen(opt->palette.mid().color());
				painter->setBrush(Qt::NoBrush);
			}
			if (opt->state.testFlag(State_Open)) {
				QPolygonF down;
				down.append(t.map(downArrow[0]));
				down.append(t.map(downArrow[1]));
				down.append(t.map(downArrow[2]));
				painter->translate(opt->rect.x() + opt->rect.width() / 2 - down.boundingRect().width() / 2,
								   opt->rect.y() + opt->rect.height() / 2 - down.boundingRect().height() / 2);
				painter->drawPolygon(down);
			} else {
				painter->translate(opt->rect.x() + opt->rect.width() / 2 - arrow.boundingRect().width() / 2,
								   opt->rect.y() + opt->rect.height() / 2 - arrow.boundingRect().height() / 2);
				painter->drawPolygon(arrow);
			}
			painter->restore();
		}
		break;
	}
	default:
		QProxyStyle::drawPrimitive(element, opt, painter, widget);
	}
}
