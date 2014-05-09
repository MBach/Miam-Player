#include "libraryscrollbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>

#include <QtDebug>

LibraryScrollBar::LibraryScrollBar(QWidget *parent)
	: ScrollBar(Qt::Vertical, parent), _hasNotEmittedYet(true)
{}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	ScrollBar::mouseMoveEvent(e);
}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mousePressEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	ScrollBar::mousePressEvent(e);
}

/** Redefined to restore covers when move events are finished. */
void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_hasNotEmittedYet) {
		emit displayItemDelegate(true);
		_hasNotEmittedYet = true;
	}
	ScrollBar::mouseReleaseEvent(e);
}
