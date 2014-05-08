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
	int xPox = mapFromGlobal(QCursor::pos()).x();
	int posButton = (float) xPox / (width() - 20) * 1000;
	qDebug() << "mousePressEvent" << xPox << width() << posButton;
	this->setValue(posButton);
	emit sliderMoved(posButton);
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

	p.setPen(o.palette.mid().color());

	//qDebug() << o.rect.x();

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
	QRectF rPlayed(QPoint(rMid.left(), rMid.top() + 1), QPoint(posButton, rMid.bottom()));

	QPainterPath path;
	path.moveTo(o.rect.x() + h, h);
	path.cubicTo(o.rect.x(), h, o.rect.x(), 2*h, o.rect.x() + h, 2*h);
	path.lineTo(rRight.bottomLeft());

	path.cubicTo(w, 2*h, w, h, w-h, h);
	//path.lineTo(rMid.topLeft());
	path.closeSubpath();
	p.fillPath(path, o.palette.base());
	p.drawPath(path);

	if (_mediaPlayer.data()->state() != QMediaPlayer::StoppedState) {
		if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
			QLinearGradient linearGradient = this->interpolatedLinearGradient(rPlayed.topLeft(), rPlayed.topRight(), o);
			p.fillRect(rPlayed, linearGradient);
		} else {
			p.fillRect(rPlayed, o.palette.mid());
		}
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
	QColor startColor = o.palette.highlight().color().lighter();
	interpolator.setStartValue(startColor);
	interpolator.setEndValue(o.palette.highlight().color());
	interpolator.setDuration(granularity) ;
	interpolator.setCurrentTime(value()) ;
	QColor c = interpolator.currentValue().value<QColor>();

	QLinearGradient linearGradient(start, end);
	linearGradient.setColorAt(0.0, startColor);
	linearGradient.setColorAt(1.0, c);

	return linearGradient;
}
