#ifndef SEARCHMEDIAPLAYERPLUGIN_H
#define SEARCHMEDIAPLAYERPLUGIN_H

#include "mediaplayerplugin.h"
#include "mediaplayer.h"

class MIAMCORE_LIBRARY SearchMediaPlayerPlugin : public MediaPlayerPlugin
{
public:
	virtual ~SearchMediaPlayerPlugin() {}
};
QT_BEGIN_NAMESPACE

#define SearchMediaPlayerPlugin_iid "MiamPlayer.SearchMediaPlayerPlugin"

Q_DECLARE_INTERFACE(SearchMediaPlayerPlugin, SearchMediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // SEARCHMEDIAPLAYERPLUGIN_H
