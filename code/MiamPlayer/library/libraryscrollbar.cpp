#include "libraryscrollbar.h"

#include <QtDebug>

LibraryScrollBar::LibraryScrollBar(QWidget *parent) :
	QScrollBar(parent), _hasNotEmittedYet(true)
{
}

void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		qDebug() << "hide covers when moving";
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mouseMoveEvent(e);
}

void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_hasNotEmittedYet) {
		qDebug() << "show covers when stopped moving";
		emit displayItemDelegate(true);
		_hasNotEmittedYet = true;
	}
	QScrollBar::mouseReleaseEvent(e);
}
