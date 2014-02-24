#include "reflector.h"

#include <QMetaEnum>
#include <QStylePainter>

#include <QtDebug>

Reflector::Reflector(QWidget *parent) :
	QWidget(parent), backgroundColor(QColor())
{}

Settings::CustomColors Reflector::customColor() const
{
	int idxEnum = Settings::staticMetaObject.indexOfEnumerator("CustomColors");
	int i = Settings::staticMetaObject.enumerator(idxEnum).keyToValue(property("CustomColors").toString().toStdString().data());
	return static_cast<Settings::CustomColors>(i);
}

/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
void Reflector::paintEvent(QPaintEvent *)
{
	QStylePainter sp(this);
	sp.setBrush(backgroundColor);
	sp.drawRect(this->rect());
}
