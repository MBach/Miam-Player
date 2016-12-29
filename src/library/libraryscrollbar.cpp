#include "libraryscrollbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QTimer>

#include <QtDebug>

LibraryScrollBar::LibraryScrollBar(QWidget *parent)
	: ScrollBar(Qt::Vertical, parent)
	, _hasNotEmittedYet(true)
	, _timer(new QTimer(this))
{
	_timer->setSingleShot(true);
	connect(_timer, &QTimer::timeout, this, [=]() {
		emit aboutToDisplayItemDelegate(false);
	});
}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit aboutToDisplayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	ScrollBar::mouseMoveEvent(e);
}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mousePressEvent(QMouseEvent *e)
{
	_timer->start(100);
	ScrollBar::mousePressEvent(e);
}

/** Redefined to restore covers when move events are finished. */
void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (_timer->isActive()) {
		_timer->stop();
	} else {
		emit aboutToDisplayItemDelegate(true);
	}
	ScrollBar::mouseReleaseEvent(e);
}
