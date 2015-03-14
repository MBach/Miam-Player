#include "seekbar.h"
#include "settingsprivate.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QWheelEvent>

#include <QtDebug>

SeekBar::SeekBar(QWidget *parent) :
	QSlider(parent)
{
	this->setMinimumHeight(30);
	this->setSingleStep(0);
	this->setPageStep(0);
}

void SeekBar::setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
}

void SeekBar::mouseMoveEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 12;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		float p = (float) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer.data()->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mousePressEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 12;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		float p = (float) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer.data()->setMute(true);
		_mediaPlayer.data()->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mouseReleaseEvent(QMouseEvent *)
{
	_mediaPlayer.data()->setMute(false);
}

/** Redefined to seek in current playing file. */
void SeekBar::wheelEvent(QWheelEvent *e)
{
	_mediaPlayer.data()->setMute(true);
	qint64 d = _mediaPlayer.data()->duration();
	if (d == 0) {
		return;
	}
	float p = _mediaPlayer.data()->position();
	qint64 currentPosition = d * p;
	qint64 s = SettingsPrivate::instance()->playbackSeekTime();
	// Wheel up is positive value, wheel down is negative value
	if (e->angleDelta().y() > 0) {
		currentPosition += s;
	} else {
		currentPosition -= s;
	}
	p = (float)currentPosition / d;
	_mediaPlayer.data()->seek(p);
	_mediaPlayer.data()->setMute(false);
}

void SeekBar::paintEvent(QPaintEvent *)
{
	int h = height() / 3.0;
	QStylePainter p(this);
	QStyleOptionSlider o;
	initStyleOption(&o);
	o.palette = QApplication::palette();

	o.rect.adjust(10, 0, -10, 0);
	static const int bound = 12;

	// Inner rectangle
	int w = width() - 2 * bound;
	float posButton = (float) value() / 1000 * w + bound;

	p.fillRect(rect(), o.palette.window());

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

	QPen pen(o.palette.mid().color());
	p.setPen(pen);
	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath ppLeft, ppRight;
	ppLeft.moveTo(rMid.topLeft());
	// 2---1---->   Left curve is painted with 2 calls to cubicTo, starting in 1
	// |   |        First cubic call is with points p1, p2 and p3
	// 3   +        Second is with points p3, p4 and p5
	// |   |        With that, a half circle can be filled with linear gradient
	// 4---5---->
	ppLeft.cubicTo(rMid.x(), rMid.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f);
	ppLeft.cubicTo(rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f,
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height(),
				   rLeft.x() + rLeft.width(), rLeft.y() + rLeft.height());

	ppRight.moveTo(rRight.topLeft());
	ppRight.cubicTo(rRight.x(), rRight.y(),
					rRight.x() + rRight.width() / 2.0f, rRight.y(),
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f);
	ppRight.cubicTo(rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f,
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height(),
					rRight.x(), rRight.y() + rRight.height());
	p.save();
	// Increase the width of the pen because of Antialising
	pen.setWidthF(1.3);
	p.setPen(pen);
	p.drawPath(ppLeft);
	p.drawPath(ppRight);
	p.restore();

	p.setRenderHint(QPainter::Antialiasing, false);
	p.drawLine(QPoint(rMid.x(), rMid.y() - 1), QPoint(rMid.x() + rMid.width(), rMid.y() - 1));
	p.drawLine(QPoint(rMid.x(), rMid.y() + rMid.height()), QPoint(rMid.x() + rMid.width(), rMid.y() + rMid.height()));
	p.restore();

	// Exclude ErrorState from painting
	if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState || _mediaPlayer.data()->state() == QMediaPlayer::PausedState) {
		//QLinearGradient linearGradient = this->interpolatedLinearGradient(rPlayed.topLeft(), rPlayed.topRight(), o);

		//p.fillPath(ppLeft, linearGradient);
		//p.fillRect(rPlayed, linearGradient);
		p.fillPath(ppLeft, Qt::red);
		p.fillRect(rPlayed, Qt::red);
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
