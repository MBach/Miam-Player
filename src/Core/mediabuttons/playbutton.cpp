#include "playbutton.h"

#include <settings.h>
#include <settingsprivate.h>

PlayButton::PlayButton(QWidget *parent)
	: MediaButton(parent)
	, _mediaPlayer(nullptr)
{

}

void PlayButton::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		auto settings = SettingsPrivate::instance();
		QString iconPath;
		if (state == QMediaPlayer::PlayingState) {
			if (settings->hasCustomIcon("pauseButton")) {
				iconPath = settings->customIcon("pauseButton");
			} else {
				iconPath = ":/player/" + Settings::instance()->theme() + "/pause";
			}
		} else {
			if (settings->hasCustomIcon("playButton")) {
				iconPath = settings->customIcon("playButton");
			} else {
				iconPath = ":/player/" + Settings::instance()->theme() + "/play";
			}
		}
		setIcon(QIcon(iconPath));
	});
}
