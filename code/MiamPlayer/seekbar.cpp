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

void SeekBar::keyPressEvent(QKeyEvent *e)
{
	_mediaPlayer.data()->blockSignals(true);
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		_mediaPlayer.data()->blockSignals(true);
		_mediaPlayer.data()->setMute(true);

		qint64 d = _mediaPlayer.data()->duration();
		if (d == 0) {
			return;
		}
		float p = _mediaPlayer.data()->position();
		qint64 currentPosition = d * p;
		qint64 s = SettingsPrivate::instance()->playbackSeekTime();

		if (e->key() == Qt::Key_Left) {
			currentPosition -= s;
		} else {
			currentPosition += s;
		}
		if (currentPosition > d || currentPosition < 0) {
			e->accept();
			return;
		}

		// qDebug() << "old position" << p << "new position" << (float)currentPosition / d;
		p = (float)currentPosition / d;
		if (p >= 0 && p <= 1.0f) {
			_mediaPlayer.data()->seek(p);
		}
		this->setValue(p * 1000);
	} else {
		QSlider::keyPressEvent(e);
	}
}

void SeekBar::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		_mediaPlayer.data()->setMute(false);
		_mediaPlayer.data()->blockSignals(false);
	} else {
		QSlider::keyPressEvent(e);
	}
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
		_mediaPlayer.data()->blockSignals(true);
		_mediaPlayer.data()->setMute(true);
		_mediaPlayer.data()->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mouseReleaseEvent(QMouseEvent *)
{
	_mediaPlayer.data()->setMute(false);
	_mediaPlayer.data()->blockSignals(false);
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

	QPen pen(o.palette.mid().color());
	p.setPen(pen);
	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath ppLeft, ppRight;
	// 2---1---->   Left curve is painted with 2 calls to cubicTo, starting in 1   <----10--9
	// |   |        First cubic call is with points p1, p2 and p3                       |   |
	// 3   +        Second is with points p3, p4 and p5                                 +   8
	// |   |        With that, a half circle can be filled with linear gradient         |   |
	// 4---5---->                                                                  <----6---7
	ppLeft.moveTo(rMid.topLeft());
	ppLeft.cubicTo(rMid.x(), rMid.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f);
	ppLeft.cubicTo(rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f,
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height(),
				   rLeft.x() + rLeft.width(), rLeft.y() + rLeft.height());

	QPainterPath pp(ppLeft);

	ppRight.moveTo(rRight.bottomLeft());
	ppRight.cubicTo(rRight.x(), rRight.y() + rRight.height(),
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height(),
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f);
	ppRight.cubicTo(rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f,
					rRight.x() + rRight.width() / 2.0f, rRight.y(),
					rRight.x(), rRight.y());

	pp.connectPath(ppRight);

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

		QLinearGradient linearGradient = this->interpolatedLinearGradient(pp.boundingRect(), o);

		p.setRenderHint(QPainter::Antialiasing, true);
		p.fillPath(pp, linearGradient);
		p.setRenderHint(QPainter::Antialiasing, false);

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

QLinearGradient SeekBar::interpolatedLinearGradient(const QRectF &boudingRect, QStyleOptionSlider &o)
{
	static QPropertyAnimation interpolator;
	interpolator.setEasingCurve(QEasingCurve::Linear);

	QColor startColor = o.palette.base().color();
	interpolator.setStartValue(startColor);
	if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
		interpolator.setEndValue(o.palette.highlight().color());
	} else {
		interpolator.setEndValue(o.palette.mid().color());
	}
	interpolator.setDuration(1000.0) ;
	interpolator.setCurrentTime(value()) ;
	QColor c = interpolator.currentValue().value<QColor>();

	QLinearGradient linearGradient(boudingRect.x(), 0, boudingRect.x() + boudingRect.width(), 0);
	linearGradient.setColorAt(0.0, startColor);
	linearGradient.setColorAt((qreal) value() / 1000, c);
	linearGradient.setColorAt((qreal) value() / 1000 + 0.001, o.palette.light().color());
	linearGradient.setColorAt(1.0, o.palette.light().color());
	return linearGradient;
}
