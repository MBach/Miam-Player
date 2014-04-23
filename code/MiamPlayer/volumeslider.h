#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>

class VolumeSlider : public QSlider
{
	Q_OBJECT
public:
	explicit VolumeSlider(QWidget *parent = 0);

protected:
	virtual void mousePressEvent(QMouseEvent *event);

	virtual void paintEvent(QPaintEvent *);

	virtual void wheelEvent(QWheelEvent *event);

};

#endif // VOLUMESLIDER_H
