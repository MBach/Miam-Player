#include "tableview.h"

#include <QGuiApplication>
#include <QScrollBar>

#include <QtDebug>

TableView::TableView(QWidget *parent) : QTableView(parent)
{
	_jumpToWidget = new JumpToWidget(this);
	this->setFrameShape(QFrame::NoFrame);
}

void TableView::setViewportMargins(int left, int top, int right, int bottom)
{
	qDebug() << Q_FUNC_INFO;
	QTableView::setViewportMargins(left, top, right, bottom);
}

void TableView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		_jumpToWidget->move(frameGeometry().right() - 19 - wVerticalScrollBar, 0);
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, 0);
	}
	QTableView::paintEvent(event);
}
