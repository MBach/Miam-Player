#include "plugininfo.h"

PluginInfo::PluginInfo(QObject *parent) :
	QObject(parent)
{

}

PluginInfo::PluginInfo(const PluginInfo &pluginInfo)
{
	_fileName = pluginInfo.fileName();
	_pluginName = pluginInfo.pluginName();
	_version = pluginInfo.version();
	_configPage = pluginInfo.configPage();
	_active = pluginInfo.isEnabled();
}

void PluginInfo::setFileName(const QString &n)
{
	_fileName = n;
}

void PluginInfo::setPluginName(const QString &p)
{
	_pluginName = p;
}

void PluginInfo::setVersion(const QString &v)
{
	_version = v;
}

void PluginInfo::setConfigPage(bool b)
{
	_configPage = b;
}

void PluginInfo::setEnabled(bool b)
{
	_active = b;
}
