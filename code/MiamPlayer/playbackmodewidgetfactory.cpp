#include "playbackmodewidgetfactory.h"

#include "settings.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QtDebug>

PlaybackModeWidgetFactory::PlaybackModeWidgetFactory(QWidget *parent, MediaButton *playbackModeButton, TabPlaylist *tabPlaylists)
	: QObject(parent), _playbackModeButton(playbackModeButton), _tabPlaylists(tabPlaylists)
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
		/// FIXME
		/*connect(i.value(), &QPushButton::clicked, [=]() {
			if (_tabPlaylists->mediaPlayer()->playlist() == NULL) {
				_tabPlaylists->mediaPlayer()->setPlaylist(_tabPlaylists->currentPlayList()->mediaPlaylist());
			}
			_tabPlaylists->mediaPlayer()->playlist()->setPlaybackMode(i.key()->mode());
		});*/
	}
}

/** Display buttons in circle (if possible, otherwise in line) around the playbackModeButton. */
void PlaybackModeWidgetFactory::move()
{
	QRect screen = QApplication::desktop()->availableGeometry();
	Edge edge = UNDEFINED;
	const int margin = 10;

	for (int index = 0; index < _popups.count(); index++) {
		PlaybackModeWidget *popup = _popups.at(index);
		qreal length = 60;
		qreal angle = (360 / _popups.count()) * index + 90;
		QLineF line = QLineF::fromPolar(length, angle);
		QPoint p = _playbackModeButton->mapToGlobal(QPoint(0, 0));
		line.translate(p);
		QPoint p2 = line.p2().toPoint();
		QRect dest = popup->rect();
		dest.translate(p2);
		if (!screen.contains(dest)) {
			if (dest.bottom() > screen.bottom()) {
				edge = BOTTOM;
			} else if (dest.right() > screen.right()) {
				edge = RIGHT;
			} else if (dest.left() < screen.left()) {
				edge = LEFT;
			} else if (dest.top() < screen.top()) {
				edge = TOP;
			}
			break;
		}
	}

	/// TODO: finish left, top, and minor init postions?
	for (int index = 0; index < _popups.count(); index++) {
		PlaybackModeWidget *popup = _popups.at(index);
		QPoint p = _playbackModeButton->mapToGlobal(QPoint(0, 0));
		QPoint p2(p);
		switch (edge) {
		case BOTTOM:
			p2.setX(p.x() - index * (popup->width() + margin));
			p2.setY(p.y() - (popup->height() + margin));
			break;
		case RIGHT:
			p2.setX(p.x() - (popup->height() + margin));
			p2.setY(p.y() - index * (popup->width() + margin));
			break;
		case LEFT:
			break;
		case TOP:
			break;
		case UNDEFINED:
			//if (_previousEdge == UNDEFINED) {
			qreal length = _playbackModeButton->frameGeometry().width() + margin;
			qreal angle = (360 / _popups.count()) * index + 90;
			QLineF line = QLineF::fromPolar(length, angle);
			line.translate(p);
			p2 = line.p2().toPoint();
			/*} else {

			}*/
			break;
		}
		if (edge == UNDEFINED || _previousEdge != UNDEFINED) {
			if (popup->isVisible()) {
				popup->move(p2);
			} else {
				popup->animate(p, p2);
			}
		} else {
			popup->animate(p, p2);
		}
	}
	_previousEdge = edge;
}

void PlaybackModeWidgetFactory::togglePlaybackModes()
{
	this->move();
	for (int i = 0; i < _popups.count(); i++) {
		PlaybackModeWidget *popup = _popups.at(i);
		popup->isVisible() ? popup->hide() : popup->show();
	}
}

void PlaybackModeWidgetFactory::update()
{
	QMediaPlaylist::PlaybackMode mode = _tabPlaylists->currentPlayList()->mediaPlaylist()->playbackMode();
	QString playbackMode = PlaybackModeWidget::nameFromMode(mode);
	_playbackModeButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/" + playbackMode));

	foreach (PlaybackModeWidget *w, _popups) {
		w->adjustIcon();
		w->adjustSize();
	}
	this->move();
}
