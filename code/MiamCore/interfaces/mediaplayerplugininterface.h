#ifndef MEDIAPLAYERPLUGININTERFACE_H
#define MEDIAPLAYERPLUGININTERFACE_H

#include "basicplugininterface.h"

#include <QMainWindow>
#include <mediaplayer.h>

class MediaPlayerPluginInterface : public BasicPluginInterface
{
public:
	virtual ~MediaPlayerPluginInterface() {}

	virtual void setMediaPlayer(QWeakPointer<MediaPlayer>) = 0;

	virtual bool providesView() const = 0;
};
QT_BEGIN_NAMESPACE

#define MediaPlayerPluginInterface_iid "MmeMiamMiamMusicPlayer.MediaPlayerPluginInterface"

Q_DECLARE_INTERFACE(MediaPlayerPluginInterface, MediaPlayerPluginInterface_iid)

QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGININTERFACE_H
