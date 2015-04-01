#include "stopbutton.h"

#include <QtDebug>

#include <QContextMenuEvent>
#include <QMenu>

StopButton::StopButton(QWidget *parent)
	: MediaButton(parent)
{
	QAction *action = _menu.addAction(tr("Stop after current"));
	action->setCheckable(true);
	connect(action, &QAction::triggered, this, [=](bool checked) {
		if (_mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
			_mediaPlayer->stopAfterCurrent(checked);
		}
	});
}

void StopButton::setMediaPlayer(QSharedPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(_mediaPlayer.data(), &MediaPlayer::currentMediaChanged, this, [=]() {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(false);
	});
}

void StopButton::contextMenuEvent(QContextMenuEvent *e)
{
	qDebug() << Q_FUNC_INFO;
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState) {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(_mediaPlayer->isStopAfterCurrent());
		_menu.exec(e->globalPos());
	}
}

