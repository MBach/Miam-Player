#include "lineedit.h"

#include <QApplication>
#include <QStylePainter>

#include <QtDebug>

LineEdit::LineEdit(QWidget *parent) :
	QLineEdit(parent), _timer(new QTimer(this)), _fps(0)
{
	this->setAttribute(Qt::WA_MacShowFocusRect, false);

	_timer->setInterval(40);
	connect(_timer, &QTimer::timeout, [=]() {
		_fps++;
		if (_fps == 25) {
			_fps = 0;
		}
		this->repaint();
	});

	_fade.setEasingCurve(QEasingCurve::Linear);
	QColor black(Qt::black), white(Qt::white);
	_fade.setStartValue(black);
	_fade.setKeyValueAt(0.5, white);
	_fade.setEndValue(black);
	_fade.setDuration(1000);
}

void LineEdit::drawCursor(QStylePainter *painter, const QRect &rText)
{
	if (hasFocus()) {
		QPoint pTop, pBottom;
		pTop = rText.topLeft();
		int fm = fontMetrics().width(text(), cursorPosition());
		pTop.rx() += fm;
		pBottom = rText.bottomLeft();
		pBottom.rx() += fm;
		_fade.setCurrentTime(_fps * _timer->interval());
		painter->setPen(_fade.currentValue().value<QColor>());
		painter->drawLine(pTop, pBottom);
	}
}

void LineEdit::focusInEvent(QFocusEvent *e)
{
	_fps = 0;
	_timer->start();
	QLineEdit::focusInEvent(e);
}

void LineEdit::focusOutEvent(QFocusEvent *e)
{
	_fps = 25;
	_timer->stop();
	this->update();
	QLineEdit::focusOutEvent(e);
}
