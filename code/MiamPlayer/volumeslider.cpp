#include "volumeslider.h"

#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <QtDebug>

VolumeSlider::VolumeSlider(QWidget *parent) :
	QSlider(parent)
{
	connect(this, &QSlider::sliderPressed, [=]() {
		qDebug() << "lambda sliderPressed" << this->value();
	});
	connect(this, &QSlider::sliderMoved, [=]() {
		qDebug() << "lambda sliderMoved" << this->value();
	});
}

void VolumeSlider::mousePressEvent(QMouseEvent *event)
{
	qDebug() << event->pos().x() << event->pos().x() * 100 / width();
	setValue(event->pos().x() * 100 / width());
	QSlider::mousePressEvent(event);
}

void VolumeSlider::paintEvent(QPaintEvent *ev)
{
	/*QStylePainter p(this);
	QStyleOptionSlider opt;
	opt.initFrom(this);
	p.drawComplexControl(QStyle::CC_Slider, opt);*/
	QSlider::paintEvent(ev);
}
