#include "playbackmodebutton.h"

#include <settings.h>
#include <settingsprivate.h>

#include <QContextMenuEvent>

#include <QtDebug>

PlaybackModeButton::PlaybackModeButton(QWidget *parent)
	: MediaButton(parent)
	, _mode(QMediaPlaylist::Sequential)
	, _toggleShuffleOnly(false)
{}

void PlaybackModeButton::setToggleShuffleOnly(bool b)
{
	if (b) {
		_mode = QMediaPlaylist::Random;
		this->setContextMenuPolicy(Qt::PreventContextMenu);
	} else {
		this->setContextMenuPolicy(Qt::DefaultContextMenu);
		connect(&_menu, &QMenu::triggered, this, [=](QAction *a) {
			QMediaPlaylist::PlaybackMode mode = (QMediaPlaylist::PlaybackMode)a->data().toInt();
			this->updateMode(mode);
		});

		connect(this, &PlaybackModeButton::clicked, this, [=]() {
			QContextMenuEvent e(QContextMenuEvent::Mouse, pos());
			this->contextMenuEvent(&e);
		});
	}
}

void PlaybackModeButton::contextMenuEvent(QContextMenuEvent *e)
{
	_menu.clear();
	QAction sequential(tr("Sequential"), nullptr);
	QAction shuffle(tr("Shuffle"), nullptr);
	QAction loop(tr("Loop"), nullptr);
	QAction itemOnce(tr("Current track once"), nullptr);
	QAction itemLoop(tr("Current track in loop"), nullptr);

	_menu.setToolTipsVisible(true);
	sequential.setData(QMediaPlaylist::Sequential);
	sequential.setToolTip(tr("Sequential mode is the most common mode to play tracks in a playlist. It will play tracks from Top to Bottom."));
	shuffle.setData(QMediaPlaylist::Random);
	loop.setData(QMediaPlaylist::Loop);
	itemOnce.setData(QMediaPlaylist::CurrentItemOnce);
	itemLoop.setData(QMediaPlaylist::CurrentItemInLoop);

	_menu.setDefaultAction(&sequential);
	_menu.addAction(&sequential);
	_menu.addAction(&shuffle);
	_menu.addAction(&loop);
	_menu.addAction(&itemOnce);
	_menu.addAction(&itemLoop);

	for (QAction *action : _menu.actions()) {
		action->setCheckable(true);
		if ((QMediaPlaylist::PlaybackMode)action->data().toInt() == _mode) {
			action->setChecked(true);
		} else {
			action->setChecked(false);
		}
	}
	_menu.exec(e->globalPos());
}

void PlaybackModeButton::setIconFromTheme(const QString &theme)
{
	QString currentMode;
	switch (_mode) {
	case QMediaPlaylist::Sequential:
		currentMode = "sequential";
		break;
	case QMediaPlaylist::Random:
		currentMode = "shuffle";
		break;
	case QMediaPlaylist::Loop:
		currentMode = "repeat";
		break;
	case QMediaPlaylist::CurrentItemOnce:
		currentMode = "itemOnce";
		break;
	case QMediaPlaylist::CurrentItemInLoop:
		currentMode = "itemLoop";
		break;
	}

	// The objectName in the UI file MUST match the alias in the QRC file!
	QString iconFile = ":/player/" + theme.toLower() + "/" + currentMode;
	if (!QFile::exists(iconFile)) {
		iconFile = ":/player/gnome/" + currentMode;
	}
	QPushButton::setIcon(QIcon(iconFile));
}

void PlaybackModeButton::updateMode(QMediaPlaylist::PlaybackMode mode)
{
	qDebug() << Q_FUNC_INFO;
	_mode = mode;
	this->setIconFromTheme(Settings::instance()->theme());
	emit aboutToChangeCurrentPlaylistPlaybackMode(_mode);
}
