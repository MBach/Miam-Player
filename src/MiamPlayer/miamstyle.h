#ifndef MIAMSTYLE_H
#define MIAMSTYLE_H

#include <QProxyStyle>

/**
 * \brief		The MiamStyle class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MiamStyle : public QProxyStyle
{
	Q_OBJECT
public:
	explicit MiamStyle(QStyle *parent = nullptr);

	virtual QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;

	virtual void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = nullptr) const override;

	virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;

	virtual void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
	void drawScrollBar(QPainter *p, const QWidget *widget) const;

};

#endif // MIAMSTYLE_H
