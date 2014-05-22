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
void MiamStyle::drawScrollBar(QPainter *p, const QWidget *widget) const
{
	QStyleOptionSlider scrollbar;
	scrollbar.palette = QApplication::palette();

	const QScrollBar *sc = qobject_cast<const QScrollBar *>(widget);
	scrollbar.initFrom(sc);


	QRect subLineRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, sc);
	QRect addLineRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, sc);
	QRect sliderRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, sc);

	qDebug() << subLineRect << sliderRect << addLineRect;


	if (sc->orientation() == Qt::Vertical) {
		subLineRect.adjust(0, 0, -1, 0);
		addLineRect.adjust(0, 0, -1, 0);
		sliderRect.adjust(0, 0, -1, 0);
	} else {
		subLineRect.adjust(0, 0, 0, -1);
		addLineRect.adjust(0, 0, 0, -1);
		sliderRect.adjust(0, 0, 0, -1);
	}

	p->setPen(Qt::NoPen);
	p->setBrush(scrollbar.palette.window());
	p->drawRect(sc->rect());

	p->setBrush(scrollbar.palette.base().color().darker(125));
	p->drawRect(sliderRect);

	// Highlight
	p->save();
	QPoint pos = sc->mapFromGlobal(QCursor::pos());
	p->setPen(scrollbar.palette.highlight().color());

	if (!sc->isSliderDown()) {
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
		//	p->drawRect(subLineRect);
		//} else if (_isDown == 1) {
		//	p->drawRect(sliderRect);
		//} else if (_isDown == 2) {
		//	p->drawRect(addLineRect);
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
	if (scrollbar.palette.windowText().color().value() < 128) {
		p->setPen(scrollbar.palette.dark().color());
		p->setBrush(scrollbar.palette.dark());
	} else {
		p->setPen(scrollbar.palette.mid().color());
		p->setBrush(scrollbar.palette.mid());
	}

	QTransform t;
	float ratio = (float) subLineRect.height() / 4.0;
	t.scale(ratio, ratio);

	if (sc->orientation() == Qt::Vertical) {
		QPolygonF up, down;
		up.append(t.map(upArrow[0]));
		up.append(t.map(upArrow[1]));
		up.append(t.map(upArrow[2]));
		down.append(t.map(downArrow[0]));
		down.append(t.map(downArrow[1]));
		down.append(t.map(downArrow[2]));
		p->translate(subLineRect.width() / 4.0, subLineRect.height() / 3.0);
		p->drawPolygon(up);
		p->translate(0, addLineRect.y());
		p->drawPolygon(down);
	} else {
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
