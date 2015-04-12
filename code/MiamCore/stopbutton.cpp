#include "stopbutton.h"
#include "mediaplayer.h"

#include <QtDebug>

#include <QContextMenuEvent>
#include <QMenu>

StopButton::StopButton(QWidget *parent)
	: MediaButton(parent)
{
	QAction *action = _menu.addAction(tr("Stop after current"));
	action->setCheckable(true);
	auto mp = MediaPlayer::instance();
	connect(action, &QAction::triggered, this, [=](bool checked) {
		if (mp->state() == QMediaPlayer::PlayingState || mp->state() == QMediaPlayer::PausedState) {
			mp->stopAfterCurrent(checked);
		}
	});
	connect(mp, &MediaPlayer::currentMediaChanged, this, [=]() {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(false);
	});
}

void StopButton::contextMenuEvent(QContextMenuEvent *e)
{
	qDebug() << Q_FUNC_INFO;
	auto mp = MediaPlayer::instance();
	if (mp->state() == QMediaPlayer::PlayingState || mp->state() == QMediaPlayer::PausedState) {
		QList<QAction*> actions = _menu.actions();
		actions.first()->setChecked(mp->isStopAfterCurrent());
		_menu.exec(e->globalPos());
	}
}

