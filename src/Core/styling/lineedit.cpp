#include "lineedit.h"

#include <QApplication>
#include <QStylePainter>

#include <QtDebug>

LineEdit::LineEdit(QWidget *parent)
	: SearchBar(parent)
	, _timer(new QTimer(this))
	, _fps(0)
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
	QPalette p = QApplication::palette();
	_fade.setStartValue(p.text().color());
	_fade.setKeyValueAt(0.5, p.base().color());
	_fade.setEndValue(p.text().color());
	_fade.setDuration(1000);

	connect(qApp, &QApplication::paletteChanged, this, [=](const QPalette &p) {
		_fade.setStartValue(p.text().color());
		_fade.setKeyValueAt(0.5, p.base().color());
		_fade.setEndValue(p.text().color());
	});
}

LineEdit::~LineEdit()
{

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
