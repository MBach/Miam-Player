#ifndef MIAMSLIDER_H
#define MIAMSLIDER_H

#include <QSlider>
#include <miamcore_global.h>

/**
 * \brief		The MiamSlider class is a style aware class.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MiamSlider : public QSlider
{
	Q_OBJECT
public:
	explicit MiamSlider(QWidget *parent = nullptr);

protected:
	virtual void paintEvent(QPaintEvent *) override;

	QLinearGradient interpolatedLinearGradient(const QRectF &boudingRect, QStyleOptionSlider &o);

private:
	void paintHorizontalSlider();

	void paintVerticalSlider();
};

#endif // MIAMSLIDER_H
