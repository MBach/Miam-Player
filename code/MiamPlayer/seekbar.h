#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QSlider>

/**
 * \brief       The SeekBar class is used to display a nice seek bar instead of default slider.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class SeekBar : public QSlider
{
    Q_OBJECT
public:
    explicit SeekBar(QWidget *parent = 0);

protected:
	virtual void mousePressEvent(QMouseEvent *event);

	virtual void paintEvent(QPaintEvent *);

private:
	QLinearGradient interpolatedLinearGradient(const QPointF &start, const QPointF &end, QStyleOptionSlider &o);
};

#endif // SEEKBAR_H
