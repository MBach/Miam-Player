#include "reflector.h"

#include <QStylePainter>

#include <QtDebug>

Reflector::Reflector(QWidget *parent) :
	QWidget(parent), backgroundColor(QColor())
{}

/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
void Reflector::paintEvent(QPaintEvent *)
{
	QStylePainter sp(this);
	sp.setBrush(backgroundColor);
	sp.drawRect(this->rect());
}
