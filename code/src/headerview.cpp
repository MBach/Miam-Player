#include "headerview.h"
#include "settings.h"

#include <QApplication>
#include <QMouseEvent>
#include <QtDebug>

HeaderView::HeaderView(Qt::Orientation orientation, QWidget * parent) :
	QHeaderView(orientation, parent)
{
	this->setStyleSheet(Settings::getInstance()->styleSheet(this));
}

bool HeaderView::event(QEvent *e)
{
	//qDebug() << "event" << e->type();
	return QHeaderView::event(e);
}

void HeaderView::mousePressEvent(QMouseEvent *e)
{
	//qDebug() << "HeaderView::mousePressEvent";
	QHeaderView::mousePressEvent(e);
}

