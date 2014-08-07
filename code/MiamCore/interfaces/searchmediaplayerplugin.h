#ifndef SEARCHMEDIAPLAYERPLUGIN_H
#define SEARCHMEDIAPLAYERPLUGIN_H

#include "mediaplayerplugin.h"
#include "mediaplayer.h"

#include <QListWidget>

class MIAMCORE_LIBRARY SearchMediaPlayerPlugin : public MediaPlayerPlugin
{
public:
	enum Request { Artist, Album, Track };

	virtual ~SearchMediaPlayerPlugin() {}

	virtual void dispatchResults(Request, QListWidget *list) = 0;

	virtual void addCheckBox(QWidget *w) = 0;

};
QT_BEGIN_NAMESPACE

#define SearchMediaPlayerPlugin_iid "MiamPlayer.SearchMediaPlayerPlugin"

Q_DECLARE_INTERFACE(SearchMediaPlayerPlugin, SearchMediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // SEARCHMEDIAPLAYERPLUGIN_H
