#ifndef MEDIAPLAYERPLUGININTERFACE_H
#define MEDIAPLAYERPLUGININTERFACE_H

#include "basicplugininterface.h"
#include "mediaplayer.h"

class MIAMCORE_LIBRARY MediaPlayerPluginInterface : public BasicPluginInterface
{
public:
	virtual ~MediaPlayerPluginInterface() {}

	virtual QWidget* providesView() = 0;

	virtual void setMediaPlayer(QWeakPointer<MediaPlayer>) = 0;
};
QT_BEGIN_NAMESPACE

#define MediaPlayerPluginInterface_iid "MiamPlayer.MediaPlayerPluginInterface"

Q_DECLARE_INTERFACE(MediaPlayerPluginInterface, MediaPlayerPluginInterface_iid)

QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGININTERFACE_H
