#ifndef TAGEDITORPLUGIN
#define TAGEDITORPLUGIN

#include "basicplugin.h"
#include "model/selectedtracksmodel.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QTableWidget>

/**
 * \brief		The TagEditorPlugin class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY TagEditorPlugin : public BasicPlugin
{
public:
	virtual ~TagEditorPlugin() {}

	virtual void setSelectedTracksModel(SelectedTracksModel *selectedTracksModel) = 0;

	virtual void setStackWidget(QStackedWidget *sw) = 0;

	virtual void setExtensibleLayout(QHBoxLayout *layout) = 0;

	virtual void setTagEditorWidget(QTableWidget *tableWidget) = 0;
};
QT_BEGIN_NAMESPACE

#define TagEditorPlugin_iid "MiamPlayer.TagEditorPlugin"

Q_DECLARE_INTERFACE(TagEditorPlugin, TagEditorPlugin_iid)

QT_END_NAMESPACE

#endif // TAGEDITORPLUGIN
