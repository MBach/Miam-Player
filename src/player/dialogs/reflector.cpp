#include "reflector.h"

#include <QApplication>
#include <QStylePainter>

Reflector::Reflector(QWidget *parent) :
	QWidget(parent), _backgroundColor(QColor())
{}

QPalette::ColorRole Reflector::colorRole() const
{
	return static_cast<QPalette::ColorRole>(property("CustomColors").toInt());
}

/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
void Reflector::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	p.setPen(QApplication::palette().mid().color());
	p.setBrush(_backgroundColor);
	p.drawRect(this->rect().adjusted(0, 0, -1, -1));
}

void Reflector::setColor(const QColor &color)
{
	this->_backgroundColor = color;
}
