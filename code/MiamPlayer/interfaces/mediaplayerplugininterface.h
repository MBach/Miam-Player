#ifndef MEDIAPLAYERPLUGININTERFACE_H
#define MEDIAPLAYERPLUGININTERFACE_H

#include "basicplugininterface.h"

#include <QMainWindow>
#include <QMediaPlayer>

class MediaPlayerPluginInterface : public BasicPluginInterface
{
public:
	virtual ~MediaPlayerPluginInterface() {}

	virtual void setMainWindow(QMainWindow *) const = 0;

	virtual void setMediaPlayer(QMediaPlayer *) const = 0;

//signals:
//	void pause();
//	void play();
//	void skip(bool forward);
//	void stop();
};
QT_BEGIN_NAMESPACE

#define MediaPlayerPluginInterface_iid "MmeMiamMiamMusicPlayer.MediaPlayerPluginInterface"

Q_DECLARE_INTERFACE(MediaPlayerPluginInterface, MediaPlayerPluginInterface_iid)

QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGININTERFACE_H
