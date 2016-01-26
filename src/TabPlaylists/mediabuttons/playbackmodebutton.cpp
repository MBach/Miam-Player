#include "playbackmodebutton.h"

#include "settingsprivate.h"

#include <QContextMenuEvent>

#include <QtDebug>

PlaybackModeButton::PlaybackModeButton(QWidget *parent)
	: MediaButton(parent)
	, _currentMode("sequential")
{
	connect(&_menu, &QMenu::triggered, this, [=](QAction *a) {
		qDebug() << Q_FUNC_INFO << a << a->data().toInt();
		QMediaPlaylist::PlaybackMode mode = (QMediaPlaylist::PlaybackMode)a->data().toInt();
		switch (mode) {
		case QMediaPlaylist::Sequential:
			_currentMode = "sequential";
			break;
		case QMediaPlaylist::Random:
			_currentMode = "shuffle";
			break;
		case QMediaPlaylist::Loop:
			_currentMode = "repeat";
			break;
		case QMediaPlaylist::CurrentItemOnce:
			_currentMode = "itemOnce";
			break;
		case QMediaPlaylist::CurrentItemInLoop:
			_currentMode = "itemLoop";
			break;
		}
	});
}

void PlaybackModeButton::contextMenuEvent(QContextMenuEvent *e)
{
	_menu.clear();
	QAction sequential(tr("Sequential"), &_menu);
	QAction shuffle(tr("Shuffle"), &_menu);
	QAction loop(tr("Loop"), &_menu);
	QAction itemOnce(tr("Current track once"), &_menu);
	QAction itemLoop(tr("Current track in loop"), &_menu);

	sequential.setData(QMediaPlaylist::Sequential);
	shuffle.setData(QMediaPlaylist::Random);
	loop.setData(QMediaPlaylist::Loop);
	itemOnce.setData(QMediaPlaylist::CurrentItemOnce);
	itemLoop.setData(QMediaPlaylist::CurrentItemInLoop);

	_menu.addAction(&sequential);
	_menu.addAction(&shuffle);
	_menu.addAction(&loop);
	_menu.addAction(&itemOnce);
	_menu.addAction(&itemLoop);

	for (QAction *action : _menu.actions()) {
		action->setCheckable(true);
		qDebug() << (QMediaPlaylist::PlaybackMode)action->data().toInt();
		if ((QMediaPlaylist::PlaybackMode)action->data().toInt() == _mode) {
			action->setChecked(true);
		}
	}
	_menu.exec(e->globalPos());
}

void PlaybackModeButton::setIconFromTheme(const QString &theme)
{
	qDebug() << Q_FUNC_INFO << theme << this->objectName() ;

	// The objectName in the UI file MUST match the alias in the QRC file!
	QString iconFile = ":/player/" + theme.toLower() + "/" + _currentMode;
	QIcon icon(iconFile);
	if (icon.isNull()) {
		iconFile = ":/player/oxygen/" + _currentMode;
	}
	QPushButton::setIcon(QIcon(iconFile));
}

void PlaybackModeButton::updateMode(QMediaPlaylist::PlaybackMode mode)
{
	qDebug() << Q_FUNC_INFO << mode;
	_mode = mode;

}
