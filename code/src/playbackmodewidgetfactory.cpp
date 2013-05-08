#include "playbackmodewidgetfactory.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QtDebug>

PlaybackModeWidgetFactory::PlaybackModeWidgetFactory(QWidget *parent, QPushButton *playbackModeButton, TabPlaylist *tabPlaylists)
	: QObject(parent), _playbackModeButton(playbackModeButton)
{
	// Create 5 buttons: one for each playback mode available
	PlaybackModeWidget *pCurrentItemOnce = new PlaybackModeWidget(QMediaPlaylist::CurrentItemOnce, playbackModeButton);
	PlaybackModeWidget *pCurrentItemLoop = new PlaybackModeWidget(QMediaPlaylist::CurrentItemInLoop, playbackModeButton);
	PlaybackModeWidget *pSequential = new PlaybackModeWidget(QMediaPlaylist::Sequential, playbackModeButton);
	PlaybackModeWidget *pLoop = new PlaybackModeWidget(QMediaPlaylist::Loop, playbackModeButton);
	PlaybackModeWidget *pRandom = new PlaybackModeWidget(QMediaPlaylist::Random, playbackModeButton);
	_popups << pCurrentItemOnce << pCurrentItemLoop << pSequential << pLoop << pRandom;

	QMap<PlaybackModeWidget*, QPushButton*> map;
	foreach (PlaybackModeWidget *popup, _popups) {
		map.insert(popup, popup->button());
	}

	QMapIterator<PlaybackModeWidget*, QPushButton*> i(map);
	while (i.hasNext()) {
		i.next();

		// When a button is clicked, it's necessary to close all other widgets
		foreach (PlaybackModeWidget *popup, _popups) {
			connect(i.value(), &QPushButton::clicked, [=]() {
				_playbackModeButton->setIcon(i.value()->icon());
				popup->close();
			});
		}

		// Also, send to the playlist the right enumeration
		connect(i.value(), &QPushButton::clicked, [=]() {
			if (tabPlaylists->mediaPlayer()->playlist() == NULL) {
				tabPlaylists->mediaPlayer()->setPlaylist(tabPlaylists->currentPlayList()->mediaPlaylist());
			}
			tabPlaylists->mediaPlayer()->playlist()->setPlaybackMode(i.key()->mode());
		});
	}
}

void PlaybackModeWidgetFactory::hideAll()
{
	for (int i = 0; i < _popups.count(); i++) {
		PlaybackModeWidget *popup = _popups.at(i);
		popup->hide();
	}
}

/** Display buttons in circle (if possible, otherwise in line) around the playbackModeButton. */
QRect PlaybackModeWidgetFactory::moveButtons(int index)
{
	/// TODO move these buttons in line when there's not enough space around the center
	qreal length = 60;
	PlaybackModeWidget *popup = _popups.at(index);
	qreal angle = (360 / _popups.count()) * index + 90;
	QLineF line = QLineF::fromPolar(length, angle);
	QPoint p = _playbackModeButton->mapToGlobal(QPoint(0, 0));
	line.translate(p);
	QPoint p2 = line.p2().toPoint();
	popup->animate(p, p2);
	return popup->frameGeometry();
}

void PlaybackModeWidgetFactory::togglePlaybackModes()
{
	for (int i = 0; i < _popups.count(); i++) {
		PlaybackModeWidget *popup = _popups.at(i);
		if (popup->isVisible()) {
			popup->hide();
		} else {
			this->moveButtons(i);
			popup->show();
		}
	}
}

/** Called when one is moving the top level window. */
/*void PlaybackModeWidgetFactory::syncPositions()
{
	QDesktopWidget *d = QApplication::desktop();

	QList<QRect> positions;
	for (int i = 0; i < _popups.count(); i++) {
		positions.append(this->moveButtons(i));
	}
	bool contains = true;
	int i = 0;
	foreach (QRect position, positions) {
		contains = contains && d->availableGeometry().contains(position);
		if (!d->availableGeometry().contains(position)) {
			//qDebug() << i << d->availableGeometry() << position;
		}
		i++;
	}
	if (contains) {
		qDebug() << "the calculated positions seems to be ok";
	} else {
		qDebug() << "the calculated positions are wrong";
		qDebug() << d->availableGeometry() << positions;
	}
}*/
