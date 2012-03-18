#include "circleprogressbar.h"

#include <QStylePainter>

#include <QtDebug>

CircleProgressBar::CircleProgressBar(QWidget *parent) :
	QProgressBar(parent), transparentCenter(false), startAngle(90.0)
{
	this->setMinimumWidth(300);
	this->setMinimumHeight(300);
	setValue(0);
}

void CircleProgressBar::paintEvent(QPaintEvent * /*event*/)
{
	//TODO
	static double coefOuter = 0.6;
	static double coefInner = 0.4;
	static double penSize = 1.0;
	static QColor borderColor(178, 178, 178);
	static QColor grooveColor(202, 202, 202);
	static QColor chunkColor(0, 211, 40);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QRectF outerRect(rect().x()*coefOuter, rect().y()*coefOuter, rect().width()*coefOuter, rect().height()*coefOuter);
	QRectF innerRect(rect().x()*coefInner, rect().y()*coefInner, rect().width()*coefInner, rect().height()*coefInner);
	outerRect.moveCenter(rect().center());
	innerRect.moveCenter(rect().center());

	if (isTextVisible()) {
		painter.save();
	}

	QPainterPath borderInAndOut;
	borderInAndOut.addEllipse(rect().center(), rect().width()/2*coefOuter, rect().height()/2*coefOuter);
	borderInAndOut.addEllipse(rect().center(), rect().width()/2*coefInner, rect().height()/2*coefInner);

	QPen borderPen(borderColor, penSize);
	painter.setPen(borderPen);
	painter.setBrush(grooveColor);
	painter.drawPath(borderInAndOut);

	if (value() > 0) {
		QPainterPath groovePath(rect().center());
		qreal converterAngle = 3.6*value();
		groovePath.arcTo(outerRect, startAngle, -converterAngle);
		groovePath.moveTo(rect().center());
		groovePath.arcTo(innerRect, startAngle, -converterAngle);
		groovePath = groovePath.simplified();
		painter.setPen(Qt::NoPen);
		painter.setBrush(chunkColor);
		painter.drawPath(groovePath);
	}

	if (!transparentCenter) {
		QPainterPath painterPathCenter;
		painterPathCenter.addEllipse(rect().center(), rect().width()/2*coefInner - penSize/2, rect().height()/2*coefInner - penSize/2);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(Qt::white));
		painter.drawPath(painterPathCenter);
	}

	if (isTextVisible()) {
		painter.restore();
		QString val = QString::number(value()).append("%");
		style()->drawItemText(&painter, rect(), Qt::AlignCenter, palette(), true, val);
	}
}

