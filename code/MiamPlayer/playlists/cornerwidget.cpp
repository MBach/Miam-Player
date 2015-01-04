#include "cornerwidget.h"

#include "settingsprivate.h"
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTab>

#include <QtDebug>

CornerWidget::CornerWidget(QWidget *parent) :
	QPushButton("", parent)
{
	this->setAcceptDrops(true);
}

void CornerWidget::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QPen plusPen;
	QPalette palette = QApplication::palette();
	QStyleOptionTab o;
	/// FIXME
	o.rect.setCoords(0, 0, 30, 20);
	int dist = 0 + SettingsPrivate::instance()->tabsOverlappingLength();
	static const qreal penScaleFactor = 0.2;

	if (this->rect().contains(mapFromGlobal(QCursor::pos()))) {
		qDebug() << "in";
		plusPen = QPen(palette.highlight(), penScaleFactor);
		p.setPen(palette.highlight().color());
		p.setBrush(palette.highlight().color().lighter());
	} else {
		qDebug() << "out";
		plusPen = QPen(palette.mid(), penScaleFactor);
		p.setPen(palette.mid().color());
		p.setBrush(palette.base());
	}
	QPainterPath pp;
	if (isLeftToRight()) {
		plusPen.setWidthF(0.75);
		// horizontal offset, diagonal offset
		const float oH = 3.0;
		const float oDiag = dist / 10;
		pp.moveTo(o.rect.x() + oDiag,
				  o.rect.y() + oH);
		pp.cubicTo(o.rect.x() + 4.0 + oDiag, o.rect.y() + o.rect.height() - (oH + 0.15 + oDiag),
				   o.rect.x() + 10.0 + oDiag, o.rect.y() + o.rect.height() - (oH + 0.1 + oDiag),
				   o.rect.x() + 7.0 + oDiag, o.rect.y() + o.rect.height() - (oH + 0.3 + oDiag));
		pp.lineTo(o.rect.x() + o.rect.width(),
				  o.rect.y() + o.rect.height() - oH - oDiag);
		pp.cubicTo(o.rect.x() + o.rect.width() - 4.0, o.rect.y() + oH + 0.15,
				   o.rect.x() + o.rect.width() - 10.0, o.rect.y() + oH + 0.1,
				   o.rect.x() + o.rect.width() - 7.0, o.rect.y() + oH + 0.3);
		pp.lineTo(o.rect.x() + oDiag,
				  o.rect.y() + oH);
	} else {
		pp.moveTo(o.rect.topRight());
		pp.cubicTo(o.rect.topRight(),
				   o.rect.bottomRight(),
				   o.rect.bottomLeft());
		pp.cubicTo(o.rect.bottomLeft(),
				   o.rect.topLeft(),
				   o.rect.topRight());
	}
	plusPen.setJoinStyle(Qt::MiterJoin);
	p.setPen(plusPen);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.drawPath(pp);
	p.setRenderHint(QPainter::Antialiasing, false);

	p.translate(o.rect.topLeft());

	// When the tabbar is very big, the inner color of [+] is a gradient like star ratings
	// Should I disable this gradient when height is small?
	p.scale(o.rect.height() * penScaleFactor, o.rect.height() * penScaleFactor);
	QLinearGradient linearGradient(0, 0, 0, o.rect.height() * 0.1);
	linearGradient.setColorAt(0, Qt::white);
	linearGradient.setColorAt(1, QColor(253, 230, 116));
	p.setBrush(linearGradient);
	//p.drawPolygon(plus, 13);
}
