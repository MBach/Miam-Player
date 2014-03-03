#include "reflector.h"

#include <QStylePainter>

#include <QtDebug>

Reflector::Reflector(QWidget *parent) :
	QWidget(parent), backgroundColor(QColor())
{}

QPalette::ColorRole Reflector::colorRole() const
{
	return static_cast<QPalette::ColorRole>(property("CustomColors").toInt());
}

/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
void Reflector::paintEvent(QPaintEvent *)
{
	QStylePainter sp(this);
	sp.setBrush(backgroundColor);
	sp.drawRect(this->rect());
}
