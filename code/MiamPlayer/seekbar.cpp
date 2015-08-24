#include "seekbar.h"
#include "settingsprivate.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QWheelEvent>

#include <QtDebug>

SeekBar::SeekBar(QWidget *parent) :
	MiamSlider(parent)
{
	this->setMinimumHeight(30);
	this->setSingleStep(0);
	this->setPageStep(0);
}

void SeekBar::keyPressEvent(QKeyEvent *e)
{
	auto mediaPlayer = MediaPlayer::instance();
	mediaPlayer->blockSignals(true);
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		mediaPlayer->blockSignals(true);
		mediaPlayer->setMute(true);
		if (e->key() == Qt::Key_Left) {
			mediaPlayer->seekBackward();
		} else {
			mediaPlayer->seekForward();
		}
	} else {
		QSlider::keyPressEvent(e);
	}
}

void SeekBar::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		MediaPlayer::instance()->setMute(false);
		MediaPlayer::instance()->blockSignals(false);
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
		MediaPlayer::instance()->seek(p);
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
		MediaPlayer::instance()->blockSignals(true);
		MediaPlayer::instance()->setMute(true);
		MediaPlayer::instance()->seek(p);
		this->setValue(posButton);
	}
}

void SeekBar::mouseReleaseEvent(QMouseEvent *)
{
	MediaPlayer::instance()->setMute(false);
	MediaPlayer::instance()->blockSignals(false);
}
