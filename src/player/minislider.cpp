#include "minislider.h"

#include <QWheelEvent>

MiniSlider::MiniSlider(QWidget *parent)
	: QSlider(parent)
{

}

void MiniSlider::mouseMoveEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 1;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		qreal p = (qreal) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer->seek(p);
		this->setValue(posButton);
	}
}

void MiniSlider::mousePressEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 1;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		qreal p = (qreal) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer->blockSignals(true);
		_mediaPlayer->setMute(true);
		_mediaPlayer->seek(p);
		this->setValue(posButton);
	}
}

void MiniSlider::mouseReleaseEvent(QMouseEvent *)
{
	_mediaPlayer->setMute(false);
	_mediaPlayer->blockSignals(false);
	if (_mediaPlayer->state() == QMediaPlayer::PausedState) {
		_mediaPlayer->togglePlayback();
	}
}

void MiniSlider::wheelEvent(QWheelEvent *e)
{
	_mediaPlayer->setMute(true);
	if (e->angleDelta().y() > 0) {
		_mediaPlayer->seekForward();
	} else {
		_mediaPlayer->seekBackward();
	}
	_mediaPlayer->setMute(false);
}

void MiniSlider::setPosition(qint64 pos, qint64 duration)
{
	if (duration > 0) {
		setValue(1000 * pos / duration);
	}
}
