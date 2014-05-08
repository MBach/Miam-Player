#include "scrollbar.h"

#include "settings.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>

#include <QtDebug>

ScrollBar::ScrollBar(QWidget *parent) :
	QScrollBar(parent)
{
}

void ScrollBar::paintEvent(QPaintEvent *e)
{
	QStylePainter p(this);
	QStyleOptionSlider scrollbar;
	initStyleOption(&scrollbar);
	scrollbar.palette = QApplication::palette();

	QRect subLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, this);
	QRect addLineRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, this);
	QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, this);

	// Draw sort indicator
	static const QPointF downArrow[3] = {
		QPointF(0.0, 0.0),
		QPointF(2.0, 0.0),
		QPointF(1.0, 1.0)
	};
	static const QPointF upArrow[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(2.0, 1.0)
	};

	p.setPen(Qt::NoPen);
	p.setBrush(scrollbar.palette.window());
	p.drawRect(this->rect());

	p.setBrush(scrollbar.palette.base().color().darker(125));
	p.drawRect(sliderRect);

	// Highlight
	p.save();
	if (subLineRect.contains(mapFromGlobal(QCursor::pos()))) {
		p.setPen(scrollbar.palette.highlight().color());
		p.setBrush(scrollbar.palette.highlight().color().lighter());
		p.drawRect(subLineRect);
	} else if (sliderRect.contains(mapFromGlobal(QCursor::pos())) || QGuiApplication::mouseButtons().testFlag(Qt::LeftButton)) {
		p.setPen(scrollbar.palette.highlight().color());
		p.setBrush(scrollbar.palette.highlight().color().lighter());
		p.drawRect(sliderRect);
	} else if (addLineRect.contains(mapFromGlobal(QCursor::pos()))) {
		p.setPen(scrollbar.palette.highlight().color());
		p.setBrush(scrollbar.palette.highlight().color().lighter());
		p.drawRect(addLineRect);
	}
	p.restore();

	// Arrows
	p.save();
	p.setPen(scrollbar.palette.base().color().darker());
	p.setBrush(scrollbar.palette.base().color().darker());
	p.translate(subLineRect.width() / 4.0, subLineRect.height() / 3.0);

	QTransform t;
	float ratio = (float) subLineRect.height() / 4.0;
	t.scale(ratio, ratio);

	QPolygonF up, down;
	up.append(t.map(upArrow[0]));
	up.append(t.map(upArrow[1]));
	up.append(t.map(upArrow[2]));
	down.append(t.map(downArrow[0]));
	down.append(t.map(downArrow[1]));
	down.append(t.map(downArrow[2]));

	p.drawPolygon(up);
	p.translate(0, addLineRect.y());
	p.drawPolygon(down);
	p.restore();
}
