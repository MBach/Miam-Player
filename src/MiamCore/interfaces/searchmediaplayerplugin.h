#ifndef SEARCHMEDIAPLAYERPLUGIN_H
#define SEARCHMEDIAPLAYERPLUGIN_H

#include "abstractsearchdialog.h"
#include "mediaplayerplugin.h"
#include "mediaplayer.h"

#include <QStandardItem>
#include <QListView>


class MIAMCORE_LIBRARY SearchMediaPlayerPlugin : public MediaPlayerPlugin
{
public:
	virtual ~SearchMediaPlayerPlugin() {}

	virtual void setSearchDialog(AbstractSearchDialog *dialog) = 0;
};
QT_BEGIN_NAMESPACE

#define SearchMediaPlayerPlugin_iid "MiamPlayer.SearchMediaPlayerPlugin"

Q_DECLARE_INTERFACE(SearchMediaPlayerPlugin, SearchMediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // SEARCHMEDIAPLAYERPLUGIN_H
