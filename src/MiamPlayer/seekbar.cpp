#include "seekbar.h"
#include "settingsprivate.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QWheelEvent>

#include <QtDebug>

SeekBar::SeekBar(QWidget *parent) :
	MiamSlider(parent), _mediaPlayer(nullptr)
{
	this->setMinimumHeight(30);
	this->setSingleStep(0);
	this->setPageStep(0);
}

void SeekBar::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
}

void SeekBar::keyPressEvent(QKeyEvent *e)
{
	_mediaPlayer->blockSignals(true);
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		_mediaPlayer->blockSignals(true);
		_mediaPlayer->setMute(true);
		if (e->key() == Qt::Key_Left) {
			_mediaPlayer->seekBackward();
		} else {
			_mediaPlayer->seekForward();
		}
	} else {
		QSlider::keyPressEvent(e);
	}
}

void SeekBar::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		_mediaPlayer->setMute(false);
		_mediaPlayer->blockSignals(false);
	} else {
		QSlider::keyPressEvent(e);
	}
}

void SeekBar::mouseMoveEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 12;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		qreal p = (qreal) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mousePressEvent(QMouseEvent *)
{
	int xPos = mapFromGlobal(QCursor::pos()).x();
	static const int bound = 12;
	if (xPos >= bound && xPos <= width() - 2 * bound) {
		qreal p = (qreal) xPos / (width() - 2 * bound);
		float posButton = p * 1000;
		_mediaPlayer->blockSignals(true);
		_mediaPlayer->setMute(true);
		_mediaPlayer->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mouseReleaseEvent(QMouseEvent *)
{
	_mediaPlayer->setMute(false);
	_mediaPlayer->blockSignals(false);
}
