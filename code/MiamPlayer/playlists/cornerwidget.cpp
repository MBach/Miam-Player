#include "cornerwidget.h"

#include "settingsprivate.h"
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTab>

#include <QtDebug>

CornerWidget::CornerWidget(TabPlaylist *parent) :
	QPushButton("", parent)
{
	this->setAcceptDrops(true);
	this->setMouseTracking(true);
	connect(this, &QPushButton::clicked, this, [=]() {
		QRect r = rect();
		if (SettingsPrivate::instance()->isRectTabs()) {
			r.setWidth(r.height());
		} else {
			r.setWidth(r.height() * 1.5);
		}
		QPoint mfg = mapFromGlobal(QCursor::pos());
		if (r.contains(mfg)) {
			emit innerButtonClicked();
		}
	});
}

void CornerWidget::mouseMoveEvent(QMouseEvent *e)
{
	QWidget::mouseMoveEvent(e);
	this->update();
}

void CornerWidget::paintEvent(QPaintEvent *)
{
	QPalette palette = QApplication::palette();
	QStylePainter p(this);
	//p.fillRect(rect(), palette.window());

	QStyleOptionTab o;
	o.initFrom(this);
	o.rect = rect();
	QPen plusPen;
	static const qreal penScaleFactor = 0.2;
	QPoint mfg = mapFromGlobal(QCursor::pos());

	p.setPen(o.palette.mid().color());
	if (SettingsPrivate::instance()->isRectTabs()) {
		p.drawLine(rect().x(), rect().y() + rect().height() - 1, rect().x() + rect().width(), rect().y() + rect().height() - 1);

		o.rect.setWidth(o.rect.height());
		float offset = o.rect.height() / 5.0;
		bool isInside = o.rect.contains(mfg);
		o.rect.adjust(offset, offset, -offset - 1, -offset - 2);
		if (isInside) {
			plusPen = QPen(palette.highlight(), penScaleFactor);
			p.setPen(palette.highlight().color());
			p.setBrush(palette.highlight().color().lighter());
		} else {
			plusPen = QPen(palette.mid(), penScaleFactor);
			p.setPen(palette.mid().color());
			p.setBrush(palette.base());
		}

		p.drawRect(o.rect);
	} else {
		o.rect.setWidth(o.rect.height() * 1.5);
		double h = this->height() / (double)8;
		bool isInside = o.rect.contains(mfg);
		o.rect.adjust(h, h, -h, -h);
		int dist = 0 + SettingsPrivate::instance()->tabsOverlappingLength();
		p.drawLine(o.rect.x() + dist - h, rect().y() + rect().height() - 1, rect().x() + rect().width(), rect().y() + rect().height() - 1);
		if (isInside) {
			plusPen = QPen(palette.highlight(), penScaleFactor);
			p.setPen(palette.highlight().color());
			p.setBrush(palette.highlight().color().lighter());
		} else {
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
	}
}
