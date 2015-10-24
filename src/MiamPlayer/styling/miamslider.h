#ifndef MIAMSLIDER_H
#define MIAMSLIDER_H

#include <QSlider>

/**
 * \brief		The MiamSlider class is a style aware class.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MiamSlider : public QSlider
{
	Q_OBJECT
public:
	explicit MiamSlider(QWidget *parent = nullptr);

protected:
	virtual void paintEvent(QPaintEvent *) override;

private:
	void paintHorizontalSlider();

	void paintVerticalSlider();

	QLinearGradient interpolatedLinearGradient(const QRectF &boudingRect, QStyleOptionSlider &o);
};

#endif // MIAMSLIDER_H
