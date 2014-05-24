#ifndef ITEMVIEWPLUGININTERFACE_H
#define ITEMVIEWPLUGININTERFACE_H

#include "basicplugininterface.h"

#include <QItemSelectionModel>
#include <QMenu>

class MIAMCORE_LIBRARY ItemViewPluginInterface : public BasicPluginInterface
{
public:
	virtual ~ItemViewPluginInterface() {}

	virtual QStringList classesToExtend() = 0;

	virtual bool hasSubMenu(const QString & /*view*/) const { return false; }

	virtual QMenu * menu(const QString & /*view*/, QMenu * /*parent*/) { return NULL; }

	virtual QAction * action(const QString & /*view*/, QMenu * /*parent*/) { return NULL; }

	virtual void setSelectionModel(const QString &view, QItemSelectionModel *) = 0;
};
QT_BEGIN_NAMESPACE

#define ItemViewPluginInterface_iid "MiamPlayer.ItemViewPluginInterface"

Q_DECLARE_INTERFACE(ItemViewPluginInterface, ItemViewPluginInterface_iid)

QT_END_NAMESPACE

#endif // ITEMVIEWPLUGININTERFACE_H
