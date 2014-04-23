#include "volumeslider.h"

#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <QtDebug>

VolumeSlider::VolumeSlider(QWidget *parent) :
	QSlider(parent)
{
	connect(this, &QSlider::sliderMoved, [=](int v) {
		this->setValue(v);
		//this->setToolTip(QString::number(value()).append("%"));
		this->update();
	});
	this->setSingleStep(5);
}

void VolumeSlider::mousePressEvent(QMouseEvent *event)
{
	//qDebug() << event->pos().x() << event->pos().x() * 100 / width();
	setValue(event->pos().x() * 100 / width());
	QSlider::mousePressEvent(event);
}

void VolumeSlider::paintEvent(QPaintEvent *)
{
	static const float bars = 10.0;
	static double y0 = 4.0 / 5.0;
	float barWidth = width() / bars;

	// Do not take all available space
	float h = height() * 0.7;

	QStylePainter p(this);
	QStyleOptionSlider opt;
	opt.initFrom(this);

	int currentVolume = floor(value() / bars);

	for (int i = 0; i < bars; i++) {
		//linear interpolation: y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
		float y = y0 - y0 * i / bars;
		//qDebug() << i << y * h << floor(h - (y * h));
		if (currentVolume >= i && value() > 0) {
			p.setPen(opt.palette.highlight().color());
			p.setBrush(opt.palette.highlight().color().lighter(100 + 100 * y));
		} else {
			p.setPen(opt.palette.mid().color());
			p.setBrush(opt.palette.midlight());
		}
		QRectF r(i * barWidth, height() * 0.15 + floor(y * h), barWidth - 2, h - floor(y * h));
		p.drawRect(r);
	}
}

void VolumeSlider::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() > 0) {
		this->setValue(value() + singleStep());
	} else {
		this->setValue(value() - singleStep());
	}
	//this->setToolTip(QString::number(value()).append("%"));
	QSlider::wheelEvent(event);
}
