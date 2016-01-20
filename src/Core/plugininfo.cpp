#include "plugininfo.h"

PluginInfo::PluginInfo(QObject *parent)
	: QObject(parent)
	, _configurable(false)
	, _active(false)
{}

/** Copy constructor required for converting in QVariant. */
PluginInfo::PluginInfo(const PluginInfo &other) :
	QObject(other.parent())
{
	_absFilePath = other.absFilePath();
	_pluginName = other.pluginName();
	_version = other.version();
	_configurable = other.isConfigurable();
	_active = other.isEnabled();
}

PluginInfo& PluginInfo::operator=(const PluginInfo& other)
{
	_absFilePath = other.absFilePath();
	_pluginName = other.pluginName();
	_version = other.version();
	_configurable = other.isConfigurable();
	_active = other.isEnabled();
	return *this;
}

/** Sets the filename. */
void PluginInfo::setAbsFilePath(const QString &n)
{
	_absFilePath = n;
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
