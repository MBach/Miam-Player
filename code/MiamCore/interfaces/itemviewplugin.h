#ifndef ITEMVIEWPLUGIN_H
#define ITEMVIEWPLUGIN_H

#include "basicplugin.h"
#include "model/selectedtracksmodel.h"

#include <QItemSelectionModel>
#include <QMenu>

class MIAMCORE_LIBRARY ItemViewPlugin : public BasicPlugin
{
public:
	virtual ~ItemViewPlugin() {}

	virtual QStringList classesToExtend() = 0;

	virtual bool hasSubMenu(const QString & /*view*/) const { return false; }

	virtual QMenu * menu(const QString & /*view*/, QMenu * /*parent*/) { return NULL; }

	virtual QAction * action(const QString & /*view*/, QMenu * /*parent*/) { return NULL; }

	virtual void setSelectedTracksModel(const QString &view, SelectedTracksModel *) = 0;
};
QT_BEGIN_NAMESPACE

#define ItemViewPlugin_iid "MiamPlayer.ItemViewPlugin"

Q_DECLARE_INTERFACE(ItemViewPlugin, ItemViewPlugin_iid)

QT_END_NAMESPACE

#endif // ITEMVIEWPLUGIN_H
