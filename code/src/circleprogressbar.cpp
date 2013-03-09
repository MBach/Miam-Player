#include "circleprogressbar.h"

#include <QStylePainter>
#include <QDebug>

CircleProgressBar::CircleProgressBar(QWidget *parent) :
	QProgressBar(parent), isTransparentCenter(true), startAngle(90.0)
{
	const int size = 400;
	this->setMinimumWidth(size);
	this->setMinimumHeight(size);
	this->setMaximumWidth(size);
	this->setMaximumHeight(size);

	static double coefOuter = 0.6;
	static double coefInner = 0.47;
	outerRect = QRectF(rect().x()*coefOuter, rect().y()*coefOuter, rect().width()*coefOuter, rect().height()*coefOuter);
	innerRect = QRectF(rect().x()*coefInner, rect().y()*coefInner, rect().width()*coefInner, rect().height()*coefInner);
	outerRect.moveCenter(rect().center());
	innerRect.moveCenter(rect().center());

	// The gray background on Windows 7
	grayRadialGradient = QRadialGradient(outerRect.center(), outerRect.width()/2, outerRect.center());
	grayRadialGradient.setSpread(QGradient::RepeatSpread);
	grayRadialGradient.setColorAt(0.0, Qt::transparent);
	grayRadialGradient.setColorAt(0.78, Qt::transparent);
	grayRadialGradient.setColorAt(0.79, QColor(213, 213, 213));
	grayRadialGradient.setColorAt(0.93, QColor(201, 201, 201));
	grayRadialGradient.setColorAt(0.95, QColor(218, 218, 218));
	grayRadialGradient.setColorAt(1.0, QColor(252, 252, 252));

	// The green groove on Windows 7
	grooveRadialGradient = QRadialGradient(outerRect.center(), outerRect.width()/2, outerRect.center());
	grooveRadialGradient.setSpread(QGradient::RepeatSpread);
	grooveRadialGradient.setColorAt(0.0, Qt::transparent);
	grooveRadialGradient.setColorAt(0.78, Qt::transparent);
	grooveRadialGradient.setColorAt(0.79, QColor(28, 226, 51));
	grooveRadialGradient.setColorAt(0.93, QColor(0, 211, 40));
	grooveRadialGradient.setColorAt(0.95, QColor(156, 238, 172));
	grooveRadialGradient.setColorAt(1.0, QColor(205, 255, 205));
}

void CircleProgressBar::paintEvent(QPaintEvent * /*event*/)
{

	static double penSize = 1.0;
	static QColor borderColor(178, 178, 178);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QPen borderPen(borderColor, penSize);

	if (!isTransparentCenter) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(Qt::white);
		painter.drawEllipse(innerRect);
	}

	// Draw the background (complete gray circle)
	painter.setPen(Qt::NoPen);
	painter.setBrush(grayRadialGradient);
	painter.drawPie(outerRect, startAngle * 16, (startAngle + 360) * 16);

	if (value() > 0) {
		// Draw the groove
		qreal convertedAngle = 3.6*value();
		painter.setPen(Qt::NoPen);
		painter.setBrush(grooveRadialGradient);
		painter.drawPie(outerRect, startAngle * 16, -convertedAngle * 16);

		if (value() < maximum()) {
			QLineF innerOuterLine(innerRect.x() + innerRect.width() / 2.0, innerRect.y(),
								  outerRect.x() + outerRect.width() / 2.0, outerRect.y());
			painter.save();
			painter.setPen(borderPen);
			painter.drawLine(innerOuterLine);
			painter.restore();
		}
	}

	// Draw 2 complete circles
	painter.setPen(borderPen);
	painter.drawArc(outerRect, startAngle * 16, (startAngle + 360) * 16);
	painter.drawArc(innerRect, startAngle * 16, (startAngle + 360) * 16);

	// Finally, draw the progress number
	if (isTextVisible()) {
		painter.setPen(Qt::black);
		QString val = QString::number(value()).append("%");
		style()->drawItemText(&painter, rect(), Qt::AlignCenter, palette(), true, val);
	}
}
