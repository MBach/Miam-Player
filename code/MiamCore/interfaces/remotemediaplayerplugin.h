#ifndef REMOTEMEDIAPLAYERPLUGIN_H
#define REMOTEMEDIAPLAYERPLUGIN_H

#include "model/sqldatabase.h"
#include "abstractsearchdialog.h"
#include "basicplugin.h"
#include "remotemediaplayer.h"

#include <QStandardItem>
#include <QListView>

class MIAMCORE_LIBRARY RemoteMediaPlayerPlugin : public BasicPlugin
{
public:
	virtual ~RemoteMediaPlayerPlugin() {}

	virtual void setSearchDialog(AbstractSearchDialog *dialog) = 0;

	virtual void setDatabase(SqlDatabase *db) = 0;

	virtual RemoteMediaPlayer * player() const = 0;

	virtual void sync(const QString &token) const = 0;
};
QT_BEGIN_NAMESPACE

#define RemoteMediaPlayerPlugin_iid "MiamPlayer.RemoteMediaPlayerPlugin"

Q_DECLARE_INTERFACE(RemoteMediaPlayerPlugin, RemoteMediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // REMOTEMEDIAPLAYERPLUGIN_H
