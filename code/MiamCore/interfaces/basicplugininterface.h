#ifndef BASICPLUGININTERFACE_H
#define BASICPLUGININTERFACE_H

#include <QStringList>
#include <QTranslator>
#include <QtPlugin>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY BasicPluginInterface
{
public:
	QTranslator translator;

	virtual ~BasicPluginInterface() {}

	virtual QStringList classesToExtend() { return QStringList(); }

	virtual void cleanUpBeforeDestroy() {}

	virtual QWidget* configPage() = 0;

	virtual void init() {}

	virtual bool isConfigurable() const = 0;

	virtual QString name() const = 0;

	virtual QString version() const = 0;
};

QT_BEGIN_NAMESPACE

#define BasicPluginInterface_iid "MiamPlayer.BasicPluginInterface"

Q_DECLARE_INTERFACE(BasicPluginInterface, BasicPluginInterface_iid)

QT_END_NAMESPACE

#endif // BASICPLUGININTERFACE_H
