#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QTimer>

/**
 * \brief		The VolumeSlider class is used to display a nice volume bar instead of default slider.
 * \details		This class shows a "classic" volume bar. It's implemented with ten large vertical rectangles increasing
 *				in height when volume gets louder. It supports the theme chosen by one and displays a gradient from light
 *				to dark, where the darkest color is the highlight color in the options.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class VolumeSlider : public QSlider
{
	Q_OBJECT
private:
	// A timer is used to hide the current value on screen
	QTimer* _timer;

	// Used to help when one has interacted with this widget
	bool _isDown;

public:
	explicit VolumeSlider(QWidget *parent = nullptr);

	/** Redefined to react to default keys */
	virtual bool eventFilter(QObject *obj, QEvent *e) override;

protected:
	/** Redefined. */
	virtual void mousePressEvent(QMouseEvent *event) override;

	/** Redefined for custom painting. */
	virtual void paintEvent(QPaintEvent *) override;

	/** Redefined to allow one to change volume without having the focus on this widget. */
	virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // VOLUMESLIDER_H
