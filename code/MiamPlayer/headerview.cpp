#include "headerview.h"

#include <QApplication>
#include <QStylePainter>

HeaderView::HeaderView(QWidget *parent) :
	QHeaderView(Qt::Horizontal, parent)
{}

void HeaderView::paintSection(QPainter *, const QRect &rect, int logicalIndex) const
{
	QStylePainter p(this->viewport());
	QStyleOptionHeader opt;
	opt.initFrom(this);
	QLinearGradient vLinearGradient(rect.topLeft(), rect.bottomLeft());
	vLinearGradient.setColorAt(0, QApplication::palette().base().color().lighter(110));
	vLinearGradient.setColorAt(1,QApplication::palette().base().color());
	p.fillRect(rect, QBrush(vLinearGradient));
	p.drawText(rect, Qt::AlignCenter, model()->headerData(logicalIndex, Qt::Horizontal).toString());

	// Frame line
	p.setPen(QApplication::palette().mid().color());
	p.drawLine(rect.bottomLeft(), rect.bottomRight());
	if (isLeftToRight() && logicalIndex == 0) {
		p.drawLine(rect.topLeft(), rect.bottomLeft());
	} else if (!isLeftToRight() && logicalIndex == count() - 1){
		p.drawLine(rect.topLeft(), rect.bottomLeft());
	}
}
