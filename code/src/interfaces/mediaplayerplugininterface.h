#ifndef MEDIAPLAYERPLUGININTERFACE_H
#define MEDIAPLAYERPLUGININTERFACE_H

#include "basicplugininterface.h"

#include <QWindow>

class MediaPlayerPluginInterface : public BasicPluginInterface
{
	Q_OBJECT
public:
	virtual ~MediaPlayerPluginInterface() {}

	virtual void setWinId(WId wId) {}

signals:
	void pause();
	void play();
	void skip(bool forward);
	void stop();
};

#define MediaPlayerPluginInterface_iid "MmeMiamMiamMusicPlayer.MediaPlayerPluginInterface"

Q_DECLARE_INTERFACE(MediaPlayerPluginInterface, MediaPlayerPluginInterface_iid)

#endif // MEDIAPLAYERPLUGININTERFACE_H
