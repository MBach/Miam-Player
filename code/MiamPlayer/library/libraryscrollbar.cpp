#include "libraryscrollbar.h"

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mouseMoveEvent(e);
}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mousePressEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mousePressEvent(e);
}

/** Redefined to restore covers when move events are finished. */
void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_hasNotEmittedYet) {
		emit displayItemDelegate(true);
		_hasNotEmittedYet = true;
	}
	QScrollBar::mouseReleaseEvent(e);
}
