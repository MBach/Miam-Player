#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <mediaplayer.h>
#include "styling/miamslider.h"

/**
 * \brief       The SeekBar class is used to display a nice seek bar instead of default slider.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class SeekBar : public MiamSlider
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	explicit SeekBar(QWidget *parent = 0);

	void setMediaPlayer(MediaPlayer *mediaPlayer);

protected:
	/** Redefined to seek in current playing file. */
	virtual void keyPressEvent(QKeyEvent *e) override;

	virtual void keyReleaseEvent(QKeyEvent *e) override;

	virtual void mouseMoveEvent(QMouseEvent *) override;

	virtual void mousePressEvent(QMouseEvent *) override;

	virtual void mouseReleaseEvent(QMouseEvent *) override;

	virtual void paintEvent(QPaintEvent *) override;
};


#endif // SEEKBAR_H
