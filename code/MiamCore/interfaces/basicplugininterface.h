#ifndef BASICPLUGININTERFACE_H
#define BASICPLUGININTERFACE_H

#include <QString>
#include <QtPlugin>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY BasicPluginInterface
{
public:
	virtual ~BasicPluginInterface() {}

	virtual QWidget* configPage() = 0;

	virtual bool isConfigurable() const = 0;

	virtual QString name() const = 0;

	virtual QString version() const = 0;
};

QT_BEGIN_NAMESPACE

#define BasicPluginInterface_iid "MmeMiamMiamMusicPlayer.BasicPluginInterface"

Q_DECLARE_INTERFACE(BasicPluginInterface, BasicPluginInterface_iid)

QT_END_NAMESPACE

#endif // BASICPLUGININTERFACE_H
