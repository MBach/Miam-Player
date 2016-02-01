#include "stopbutton.h"

#include <QAction>
#include <QContextMenuEvent>

#include <QtDebug>

StopButton::StopButton(QWidget *parent)
	: MediaButton(parent)
	, _mediaPlayer(nullptr)
{
	_action = _menu.addAction(tr("Stop after current"));
	_action->setCheckable(true);
}

void StopButton::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(_action, &QAction::triggered, this, [=](bool checked) {
		if (mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
			mediaPlayer->stopAfterCurrent(checked);
		}
	});
	connect(mediaPlayer, &MediaPlayer::currentMediaChanged, this, [=]() {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(false);
	});
}

void StopButton::contextMenuEvent(QContextMenuEvent *e)
{
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(_mediaPlayer->isStopAfterCurrent());
		_menu.exec(e->globalPos());
	}
}

