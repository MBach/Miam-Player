#ifndef MEDIAPLAYERPLUGIN_H
#define MEDIAPLAYERPLUGIN_H

#include "basicplugin.h"
#include "mediaplayer.h"

class MIAMCORE_LIBRARY MediaPlayerPlugin : public BasicPlugin
{
public:
	virtual ~MediaPlayerPlugin() {}

	virtual QWidget* providesView() = 0;

	virtual void setMediaPlayer(QWeakPointer<MediaPlayer>) = 0;
};
QT_BEGIN_NAMESPACE

#define MediaPlayerPlugin_iid "MiamPlayer.MediaPlayerPlugin"

Q_DECLARE_INTERFACE(MediaPlayerPlugin, MediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGIN_H
