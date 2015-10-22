#include "miamslider.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <QtDebug>

MiamSlider::MiamSlider(QWidget *parent)
	: QSlider(parent)
{}

void MiamSlider::paintHorizontalSlider()
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
	/// FIXME
	//if (MediaPlayer::instance()->state() == QMediaPlayer::PlayingState || MediaPlayer::instance()->state() == QMediaPlayer::PausedState) {

		QLinearGradient linearGradient = this->interpolatedLinearGradient(pp.boundingRect(), o);

		p.setRenderHint(QPainter::Antialiasing, true);
		p.fillPath(pp, linearGradient);
		p.setRenderHint(QPainter::Antialiasing, false);

		p.save();
		p.setRenderHint(QPainter::Antialiasing, true);
		QPointF center(posButton, height() * 0.5);
		QConicalGradient cGrad(center, 360 - 4 * (value() % 360));
		//if (MediaPlayer::instance()->state() == QMediaPlayer::PlayingState) {
			cGrad.setColorAt(0.0, o.palette.highlight().color());
			cGrad.setColorAt(1.0, o.palette.highlight().color().lighter());
		//} else {
		//	cGrad.setColorAt(0.0, o.palette.mid().color());
		//	cGrad.setColorAt(1.0, o.palette.mid().color().lighter());
		//}
		p.setBrush(cGrad);
		p.drawEllipse(center, height() * 0.3, height() * 0.3);
		p.restore();
	//}
}

void MiamSlider::paintVerticalSlider()
{
	int w = width() / 3.0;
	w = qMin(w, 10);

	QStylePainter p(this);
	QStyleOptionSlider o;
	initStyleOption(&o);
	o.palette = QApplication::palette();

	o.rect.adjust(0, 10, 0, -10);
	const int bound = 15;

	// Inner rectangle
	int h = height() - 2 * bound;

	p.fillRect(rect(), o.palette.window());


	int x = (width() - w) / 2 + o.rect.x();
	QRectF rTop = QRectF(x, o.rect.y(), w, w);
	QRectF rBottom = QRectF(x, o.rect.y() + o.rect.height() - w, w, w);
	QRectF rMid = QRectF(rTop.bottomLeft(), rBottom.topRight());

	QPen pen(o.palette.mid().color());
	p.setPen(pen);
	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath ppTop, ppBottom;

	// 4---3---2    Top curve is painted with 2 calls to cubicTo, starting in 1     ^       ^
	// |       |    First cubic call is with points p1, p2 and p3                   |       |
	// 5---+---1    Second is with points p3, p4 and p5                             6---+--10
	// |       |    With that, a half circle can be filled with linear gradient     |       |
	// \/      \/                                                                   7---8---9
	ppTop.moveTo(rMid.topRight());
	ppTop.cubicTo(rTop.x() + rTop.width(), rMid.y(),
				  rTop.x() + rTop.width(), rTop.y() + rTop.height() / 2.0f,
				  rTop.x() + rTop.width() / 2.0f, rTop.y() + rTop.height() / 2.0f);
	ppTop.cubicTo(rTop.x() + rTop.width() / 2.0f, rTop.y() + rTop.height() / 2.0f,
				  rTop.x(), rTop.y() + rTop.height() / 2.0f,
				  rTop.x(), rTop.y() + rTop.height());

	QPainterPath pp(ppTop);

	ppBottom.moveTo(rBottom.topLeft());
	ppBottom.cubicTo(rBottom.x(), rBottom.y(),
					 rBottom.x(), rBottom.y() + rBottom.height() / 2.0f,
					 rBottom.x() + rBottom.width() / 2.0f, rBottom.y() + rBottom.height() / 2.0f);
	ppBottom.cubicTo(rBottom.x() + rBottom.width() / 2.0f, rBottom.y() + rBottom.height() / 2.0f,
					 rBottom.x() + rBottom.width(), rBottom.y() + rBottom.height() / 2.0f,
					 rBottom.x() + rBottom.width(), rBottom.y());

	pp.connectPath(ppBottom);

	p.save();
	// Increase the width of the pen because of Antialising
	pen.setWidthF(1.3);
	p.setPen(pen);
	p.drawPath(ppTop);
	p.drawPath(ppBottom);
	p.restore();

	p.setRenderHint(QPainter::Antialiasing, false);
	p.drawLine(QPoint(rMid.x() - 1, rMid.y() - 1), QPoint(rMid.x() - 1, rMid.y() + rMid.height() - 1));
	p.drawLine(QPoint(rMid.x() + rMid.width(), rMid.y() - 1), QPoint(rMid.x() + rMid.width(), rMid.y() + rMid.height() - 1));
	p.restore();

	QLinearGradient linearGradient = this->interpolatedLinearGradient(pp.boundingRect(), o);

	p.setRenderHint(QPainter::Antialiasing, true);
	p.fillPath(pp, linearGradient);
	p.setRenderHint(QPainter::Antialiasing, false);

	p.save();
	p.setRenderHint(QPainter::Antialiasing, true);
	int total = maximum() - minimum() + 1;
	int v = value() - minimum();
	float posButton = h - (float) v / total * h + bound;
	QPointF center(rMid.center().x(), posButton);
	QConicalGradient cGrad(center, 360 - 4 * (value() % 360));
	if (o.state.testFlag(QStyle::State_Enabled)) {
		cGrad.setColorAt(0.0, o.palette.highlight().color());
		cGrad.setColorAt(1.0, o.palette.highlight().color().lighter());
	} else {
		cGrad.setColorAt(0.0, o.palette.mid().color());
		cGrad.setColorAt(1.0, o.palette.mid().color().lighter());
	}
	p.setBrush(cGrad);
	p.drawEllipse(center, w, w);
	p.restore();
}

void MiamSlider::paintEvent(QPaintEvent *)
{
	if (orientation() == Qt::Horizontal) {
		this->paintHorizontalSlider();
	} else {
		this->paintVerticalSlider();
	}
}

QLinearGradient MiamSlider::interpolatedLinearGradient(const QRectF &boudingRect, QStyleOptionSlider &o)
{
	QPropertyAnimation interpolator;
	interpolator.setEasingCurve(QEasingCurve::Linear);

	QColor startColor = o.palette.base().color();
	interpolator.setStartValue(startColor);
	if (o.state.testFlag(QStyle::State_Enabled)) {
		interpolator.setEndValue(o.palette.highlight().color());
	} else {
		interpolator.setEndValue(o.palette.mid().color());
	}
	int total = maximum() - minimum() + 1;
	interpolator.setDuration(total) ;
	interpolator.setCurrentTime(value() - minimum()) ;
	QColor c = interpolator.currentValue().value<QColor>();

	QLinearGradient linearGradient;
	if (orientation() == Qt::Horizontal) {
		linearGradient.setStart(boudingRect.x(), 0);
		linearGradient.setFinalStop(boudingRect.x() + boudingRect.width(), 0);
	} else {
		linearGradient.setStart(0, boudingRect.y() + boudingRect.height());
		linearGradient.setFinalStop(0, boudingRect.y());
	}
	linearGradient.setColorAt(0.0, startColor);
	linearGradient.setColorAt((qreal) (value() - minimum()) / total, c);
	linearGradient.setColorAt((qreal) (value() - minimum()) / total + 0.001, o.palette.light().color());
	linearGradient.setColorAt(1.0, o.palette.light().color());
	return linearGradient;
}
