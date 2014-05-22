#include "seekbar.h"
#include "settings.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <QtDebug>

SeekBar::SeekBar(QWidget *parent) :
    QSlider(parent)
{
	this->setMinimumHeight(30);
}

void SeekBar::mousePressEvent(QMouseEvent *event)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 12;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		qDebug() << "mousePressEvent inside clickable area";
		int posButton = (float) xPos / (width() - 2 * bound) * 1000;
		qDebug() << "mousePressEvent" << "xPos" << xPos << "width()" << width() << "posButton" << posButton;
		//this->setValue(posButton);
		emit sliderPressed();
		QSlider::mousePressEvent(event);
		this->repaint();
	} /*else {
		qDebug() << "mousePressEvent OUTSIDE clickable area";
	}*/
}

void SeekBar::mouseReleaseEvent(QMouseEvent *e)
{
	emit sliderMoved(value());
	QSlider::mouseReleaseEvent(e);
}

void SeekBar::paintEvent(QPaintEvent *)
{
	//qDebug() << "paintEvent" << value();
	static const int h = height() / 3.0;
	QStylePainter p(this);
	QStyleOptionSlider o;
	initStyleOption(&o);
	o.palette = QApplication::palette();

	o.rect.adjust(10, 0, -10, 0);
	static const int bound = 12;
	int w = width() - 2 * bound;
	float posButton = (float) value() / 1000 * w;

	p.fillRect(rect(), o.palette.window());

	static const int startAngle = 90 * 16;
	static const int spanAngle = 180 * 16;

	QRectF rLeft = QRectF(o.rect.x(),
						o.rect.y() + h,
						h,
						h);
	QRectF rRight = QRectF(o.rect.x() + o.rect.width() - h,
						 h,
						 h,
						 h);
	QRectF rMid = QRectF(rLeft.topRight(), rRight.bottomLeft());

	// Fill the seek bar with highlighted color
	QRectF rPlayed(QPoint(rMid.left(), rMid.top()), QPoint(posButton, rMid.bottom()));

	p.setPen(o.palette.mid().color());
	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	p.drawArc(rLeft, startAngle, spanAngle - 16);
	p.drawArc(rRight, (180 * 16) + startAngle, spanAngle);
	p.setRenderHint(QPainter::Antialiasing, false);
	p.drawLine(QPoint(rLeft.center().x(), rLeft.y() - 1), QPoint(rRight.center().x() - 1, rRight.y() - 1));
	p.drawLine(QPoint(rLeft.center().x(), rLeft.bottom()), QPoint(rRight.center().x() - 1, rRight.bottom()));
	p.restore();

	if (_mediaPlayer.data()->state() != QMediaPlayer::StoppedState) {
		QLinearGradient linearGradient = this->interpolatedLinearGradient(rPlayed.topLeft(), rPlayed.topRight(), o);
		p.fillRect(rPlayed, linearGradient);
		p.save();
		p.setRenderHint(QPainter::Antialiasing, true);
		QPointF center(posButton, height() * 0.5);
		QConicalGradient cGrad(center, 360 - 4 * (value() % 360));
		if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
			cGrad.setColorAt(0.0, o.palette.highlight().color());
			cGrad.setColorAt(1.0, o.palette.highlight().color().lighter());
		} else {
			cGrad.setColorAt(0.0, o.palette.mid().color());
			cGrad.setColorAt(1.0, o.palette.mid().color().lighter());
		}
		p.setBrush(cGrad);
		p.drawEllipse(center, height() * 0.3, height() * 0.3);
		p.restore();
	}
}

QLinearGradient SeekBar::interpolatedLinearGradient(const QPointF &start, const QPointF &end, QStyleOptionSlider &o)
{
	QPropertyAnimation interpolator;
	interpolator.setEasingCurve(QEasingCurve::Linear);

	const qreal granularity = 1000.0;
	/// FIXME: drawChord(rectLeft) if converted(value() -> posX) in rectLeft
	//QColor startColor = o.palette.highlight().color().lighter();
	QColor startColor = o.palette.base().color();
	interpolator.setStartValue(startColor);
	if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
		interpolator.setEndValue(o.palette.highlight().color());
	} else {
		interpolator.setEndValue(o.palette.mid().color());
	}
	interpolator.setDuration(granularity) ;
	interpolator.setCurrentTime(value()) ;
	QColor c = interpolator.currentValue().value<QColor>();

	QLinearGradient linearGradient(start, end);
	linearGradient.setColorAt(0.0, startColor);
	linearGradient.setColorAt(1.0, c);

	return linearGradient;
}
