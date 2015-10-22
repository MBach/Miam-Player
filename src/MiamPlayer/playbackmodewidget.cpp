#include "playbackmodewidget.h"
#include "settings.h"
#include "settingsprivate.h"

/// FIXME: use Qt::Popup to avoir flicker effect when one clicks. The problem is to manage multiple popups on screen
/// It seems difficult to handle more than one popup
PlaybackModeWidget::PlaybackModeWidget(QMediaPlaylist::PlaybackMode mode, QPushButton *playbackModeButton) :
	QWidget(playbackModeButton, Qt::Tool | Qt::FramelessWindowHint), _mode(mode)
{
	_animation = new QPropertyAnimation(this, "pos");
	_animation->setDuration(200);

	QPushButton *button = new QPushButton(this);
	_playbackMode = PlaybackModeWidget::nameFromMode(mode);

	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	Settings *settings = Settings::instance();
	button->setIcon(QIcon(":/player/" + settings->theme() + "/" + _playbackMode));
	button->setIconSize(QSize(settingsPrivate->buttonsSize(), settingsPrivate->buttonsSize()));

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->addWidget(button);
	this->setLayout(vLayout);
}

/** Convert Enum in QString to dynamically load icons. */
QString PlaybackModeWidget::nameFromMode(QMediaPlaylist::PlaybackMode mode)
{
	QString playbackMode;
	switch (mode) {
	case QMediaPlaylist::CurrentItemOnce:
		playbackMode = "itemOnce";
		break;
	case QMediaPlaylist::CurrentItemInLoop:
		playbackMode = "itemLoop";
		break;
	case QMediaPlaylist::Sequential:
		playbackMode = "sequential";
		break;
	case QMediaPlaylist::Loop:
		playbackMode = "repeat";
		break;
	case QMediaPlaylist::Random:
		playbackMode = "shuffle";
		break;
	}
	return playbackMode;
}

/** Reload icon when theme has changed or buttons size was changed in options by one. */
void PlaybackModeWidget::adjustIcon()
{
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	Settings *settings = Settings::instance();
	button()->setIcon(QIcon(":/player/" + settings->theme() + "/" + _playbackMode));
	button()->setIconSize(QSize(settingsPrivate->buttonsSize(), settingsPrivate->buttonsSize()));
}

/** Animates this button in circle or in line. */
void PlaybackModeWidget::animate(const QPoint &start, const QPoint &end)
{
	_animation->setStartValue(start);
	_animation->setEndValue(end);
	_animation->start();
}
