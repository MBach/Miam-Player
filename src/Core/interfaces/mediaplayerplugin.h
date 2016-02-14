#ifndef MEDIAPLAYERPLUGIN_H
#define MEDIAPLAYERPLUGIN_H

#include "basicplugin.h"
#include "mediaplayer.h"
#include "mediaplayercontrol.h"

class AbstractView;

/**
 * \brief		The MediaPlayerPlugin class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaPlayerPlugin : public BasicPlugin
{
	Q_OBJECT
public:
	MediaPlayerPlugin(QObject *parent = nullptr) : BasicPlugin(parent) {}

	virtual ~MediaPlayerPlugin() {}

	virtual bool hasView() const = 0;

	virtual AbstractView* instanciateView() { return nullptr; }

	//virtual void setMediaPlayer(MediaPlayer *) = 0;

	virtual void setMediaPlayerControl(MediaPlayerControl *) = 0;

	virtual QStringList extensions() const = 0;
};
QT_BEGIN_NAMESPACE

#define MediaPlayerPlugin_iid "MiamPlayer.MediaPlayerPlugin"

Q_DECLARE_INTERFACE(MediaPlayerPlugin, MediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // MEDIAPLAYERPLUGIN_H
