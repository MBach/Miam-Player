#ifndef MINISLIDER_H
#define MINISLIDER_H

#include <QSlider>
#include <mediaplayer.h>

class MiniSlider : public QSlider
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	explicit MiniSlider(QWidget *parent = nullptr);

	inline void setMediaPlayer(MediaPlayer *mediaPlayer) { _mediaPlayer = mediaPlayer; }

protected:
	virtual void mouseMoveEvent(QMouseEvent *) override;

	virtual void mousePressEvent(QMouseEvent *) override;

	virtual void mouseReleaseEvent(QMouseEvent *) override;

	virtual void wheelEvent(QWheelEvent *e) override;

public slots:
	void setPosition(qint64 pos, qint64 duration);
};

#endif // MINISLIDER_H
