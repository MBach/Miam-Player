#ifndef MIAMSTYLE_H
#define MIAMSTYLE_H

#include <QProxyStyle>

class MiamStyle : public QProxyStyle
{
	Q_OBJECT
public:
	explicit MiamStyle(QStyle *parent = 0);

	QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

	//virtual void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const;

	virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

	virtual void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
	void drawScrollBar(QPainter *p, const QWidget *widget) const;

};

#endif // MIAMSTYLE_H
