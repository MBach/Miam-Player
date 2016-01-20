#ifndef REMOTEMEDIAPLAYERPLUGIN_H
#define REMOTEMEDIAPLAYERPLUGIN_H

#include "basicplugin.h"
#include "imediaplayer.h"

#include <QStandardItem>
#include <QListView>

/// Forward declaration
class AbstractSearchDialog;

/**
 * \brief		The RemoteMediaPlayerPlugin class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY RemoteMediaPlayerPlugin : public BasicPlugin
{
	Q_OBJECT
public:
	explicit RemoteMediaPlayerPlugin(QObject *parent = nullptr) : BasicPlugin(parent) {}

	virtual ~RemoteMediaPlayerPlugin() {}

	virtual void setSearchDialog(AbstractSearchDialog *dialog) = 0;

	virtual IMediaPlayer * player() const = 0;

	virtual void sync(const QString &token) const = 0;
};
QT_BEGIN_NAMESPACE

#define RemoteMediaPlayerPlugin_iid "MiamPlayer.RemoteMediaPlayerPlugin"

Q_DECLARE_INTERFACE(RemoteMediaPlayerPlugin, RemoteMediaPlayerPlugin_iid)

QT_END_NAMESPACE

#endif // REMOTEMEDIAPLAYERPLUGIN_H
