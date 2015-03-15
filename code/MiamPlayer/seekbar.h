#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QSlider>

#include "mediaplayer.h"

/**
 * \brief       The SeekBar class is used to display a nice seek bar instead of default slider.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class SeekBar : public QSlider
{
	Q_OBJECT
private:
	/** Reference to the unique instance of MediaPlayer class in the application. */
	QWeakPointer<MediaPlayer> _mediaPlayer;

public:
	explicit SeekBar(QWidget *parent = 0);

	void setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer);

protected:
	/** Redefined to seek in current playing file. */
	virtual void keyPressEvent(QKeyEvent *e) override;

	virtual void keyReleaseEvent(QKeyEvent *e) override;

	virtual void mouseMoveEvent(QMouseEvent *) override;

	virtual void mousePressEvent(QMouseEvent *) override;

	virtual void mouseReleaseEvent(QMouseEvent *) override;

	/** Redefined to seek in current playing file. */
	virtual void wheelEvent(QWheelEvent *e) override;

	virtual void paintEvent(QPaintEvent *) override;

private:
	QLinearGradient interpolatedLinearGradient(const QRectF &boudingRect, QStyleOptionSlider &o);
};


#endif // SEEKBAR_H
