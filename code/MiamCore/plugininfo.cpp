#include "plugininfo.h"

PluginInfo::PluginInfo(QObject *parent) :
	QObject(parent)
{}

/** Copy constructor required for converting in QVariant. */
PluginInfo::PluginInfo(const PluginInfo &pluginInfo) :
	QObject(pluginInfo.parent())
{
	_fileName = pluginInfo.fileName();
	_pluginName = pluginInfo.pluginName();
	_version = pluginInfo.version();
	_configurable = pluginInfo.isConfigurable();
	_active = pluginInfo.isEnabled();
}

/** Sets the filename. */
void PluginInfo::setFileName(const QString &n)
{
	_fileName = n;
}

/** Sets the name. */
void PluginInfo::setPluginName(const QString &p)
{
	_pluginName = p;
}

/** Sets the version. */
void PluginInfo::setVersion(const QString &v)
{
	_version = v;
}

/** Sets if the plugin has its own config page. */
void PluginInfo::setConfigPage(bool b)
{
	_configurable = b;
}

/** Sets if the plugin is enabled. */
void PluginInfo::setEnabled(bool b)
{
	_active = b;
}
