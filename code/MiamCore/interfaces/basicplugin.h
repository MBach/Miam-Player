#ifndef BASICPLUGIN_H
#define BASICPLUGIN_H

#include <QStringList>
#include <QTranslator>
#include <QtPlugin>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY BasicPlugin
{
public:
	QTranslator translator;

	virtual ~BasicPlugin() {}

	virtual QStringList classesToExtend() { return QStringList(); }

	virtual void cleanUpBeforeDestroy() {}

	virtual QWidget* configPage() = 0;

	virtual void init() {}

	virtual bool isConfigurable() const = 0;

	virtual QString name() const = 0;

	virtual QString version() const = 0;
};

QT_BEGIN_NAMESPACE

#define BasicPlugin_iid "MiamPlayer.BasicPlugin"

Q_DECLARE_INTERFACE(BasicPlugin, BasicPlugin_iid)

QT_END_NAMESPACE

#endif // BASICPLUGIN_H
