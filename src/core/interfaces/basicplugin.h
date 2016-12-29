#ifndef BASICPLUGIN_H
#define BASICPLUGIN_H

#include <QStringList>
#include <QTranslator>
#include <QtPlugin>

#include "miamcore_global.h"

class MusicSearchEngine;

/**
 * \brief		The BasicPlugin class is the base class for creating a plugin.
 * \details     Derived classes that will use this interface must reimplement almost everything (name, version, etc).
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY BasicPlugin : public QObject
{
	Q_OBJECT
public:
	QTranslator translator;

	explicit BasicPlugin(QObject *parent = nullptr) : QObject(parent) {}

	/** Default desctructor. */
	virtual ~BasicPlugin() {}

	virtual bool canInteractWithSearchEngine() const { return false; }

	/** This method can specify some classes to extend at runtime, like QMenu (for appending new items). */
	virtual QStringList classesToExtend() { return QStringList(); }

	virtual void cleanUpBeforeDestroy() {}

	/** This Widget is instanciated in settings and appended to the list of plugin which can be customized at runtime. */
	virtual QWidget* configPage() = 0;

	virtual void init() {}

	/** Must return true if this plugin provides a config page. */
	virtual bool isConfigurable() const = 0;

	virtual void setMusicSearchEngine(MusicSearchEngine *) {}

	/** Name of plugin displayed in settings. */
	virtual QString name() const = 0;

	/** Version of this plugin. */
	virtual QString version() const = 0;
};

QT_BEGIN_NAMESPACE

#define BasicPlugin_iid "MiamPlayer.BasicPlugin"

Q_DECLARE_INTERFACE(BasicPlugin, BasicPlugin_iid)

QT_END_NAMESPACE

#endif // BASICPLUGIN_H
