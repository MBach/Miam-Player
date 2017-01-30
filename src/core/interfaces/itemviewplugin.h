#ifndef ITEMVIEWPLUGIN_H
#define ITEMVIEWPLUGIN_H

#include "basicplugin.h"
#include "model/selectedtracksmodel.h"

#include <QItemSelectionModel>
#include <QMenu>

/**
 * \brief		The ItemViewPlugin class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY ItemViewPlugin : public BasicPlugin
{
	Q_OBJECT
public:
	explicit ItemViewPlugin(QObject *parent = nullptr) : BasicPlugin(parent) {}

	virtual ~ItemViewPlugin() {}

	virtual bool hasSubMenu(const QString & /*view*/) const { return false; }

	virtual QMenu * menu(const QString & /*view*/, QMenu * /*parent*/) { return nullptr; }

	virtual QAction * action(const QString & /*view*/, QMenu * /*parent*/) { return nullptr; }

	virtual void setSelectedTracksModel(const QString &view, SelectedTracksModel *) = 0;
};
QT_BEGIN_NAMESPACE

#define ItemViewPlugin_iid "MiamPlayer.ItemViewPlugin"

Q_DECLARE_INTERFACE(ItemViewPlugin, ItemViewPlugin_iid)

QT_END_NAMESPACE

#endif // ITEMVIEWPLUGIN_H
