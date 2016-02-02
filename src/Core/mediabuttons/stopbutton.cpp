#include "stopbutton.h"

#include <QAction>
#include <QContextMenuEvent>

#include <QtDebug>

StopButton::StopButton(QWidget *parent)
	: MediaButton(parent)
	, _mediaPlayer(nullptr)
{}

void StopButton::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(&_menu, &QMenu::triggered, this, [=](QAction *action) {
		if (mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
			mediaPlayer->stopAfterCurrent(action->isChecked());
		}
	});
	connect(mediaPlayer, &MediaPlayer::currentMediaChanged, this, [=]() {
		QList<QAction*> actions = _menu.actions();
		if (!actions.isEmpty()) {
			actions.first()->setChecked(false);
		}
	});
}

void StopButton::contextMenuEvent(QContextMenuEvent *e)
{
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
		if (_mediaPlayer->isStopAfterCurrent()) {
			QList<QAction*> actions = _menu.actions();
			if (!actions.isEmpty()) {
				actions.first()->setChecked(true);
			}
		} else {
			_menu.clear();
			QAction *stopAfterCurrentAction = _menu.addAction(tr("Stop after current"));
			stopAfterCurrentAction->setCheckable(true);
		}
		_menu.exec(e->globalPos());
	}
}
