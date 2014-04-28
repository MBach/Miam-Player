#include "seekbar.h"
#include "settings.h"

#include <QApplication>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <QtDebug>

SeekBar::SeekBar(QWidget *parent) :
    QSlider(parent)
{
	this->setMinimumHeight(30);
	//this->setValue(500);
}

void SeekBar::mousePressEvent(QMouseEvent *event)
{
	int xPox = mapFromGlobal(QCursor::pos()).x();
	int posButton = (float) xPox / width() * 1000;
	qDebug() << "mousePressEvent" << xPox << width() << posButton;
	this->setValue(posButton);
	emit valueChanged(posButton);
	QSlider::mousePressEvent(event);
}

void SeekBar::paintEvent(QPaintEvent *)
{
	static const int h = height() / 3.0;
	QStylePainter p(this);
	QStyleOptionSlider o;
	initStyleOption(&o);
	o.palette = QApplication::palette();
	o.rect.adjust(10, 0, -10, 0);
	int w = o.rect.width();
	float posButton = (float) value() / 1000 * w;
	//qDebug() << posButton;

	QPen pen(o.palette.mid().color(), 0.66);
	p.setPen(pen);
	//p.setPen(o.palette.mid().color());

	//static const int startAngle = 90 * 16;
	//static const int spanAngle = 180 * 16;

	qDebug() << o.rect.x();

	QRectF rLeft = QRectF(o.rect.x(),
						o.rect.y() + h,
						h,
						h);
	QRectF rRight = QRectF(o.rect.x() + o.rect.width() - h,
						 h,
						 h,
						 h);
	QRectF rMid = QRectF(rLeft.topRight(), rRight.bottomLeft());

	//p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath path;
	path.moveTo(o.rect.x() + h, h);
	path.cubicTo(o.rect.x(), h, o.rect.x(), 2*h, o.rect.x() + h, 2*h);
	path.lineTo(rRight.bottomLeft());

	path.cubicTo(w, 2*h, w, h, w-h, h);
	//path.lineTo(rMid.topLeft());
	path.closeSubpath();
	p.fillPath(path, o.palette.base());
	p.drawPath(path);

	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	pen.setWidth(1.0);
	QPointF center(posButton, height() * 0.5);
	QConicalGradient cGrad(center, 0.0);
	cGrad.setColorAt(0.0, o.palette.highlight().color());
	cGrad.setColorAt(1.0, o.palette.highlight().color().lighter());
	p.setBrush(cGrad);
	p.drawEllipse(center, height() * 0.3, height() * 0.3);
	p.restore();
}
