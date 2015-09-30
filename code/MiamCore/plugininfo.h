#ifndef PLUGININFO_H
#define PLUGININFO_H

#include <QDataStream>
#include <QObject>
#include "miamcore_global.h"

/**
 * \brief		The PluginInfo class is a Data Access Object
 * \details     This class is used to keep track of one's settings. It's necessary to be able to have a Widget with one row
 *		for each plugin, to display few informations like its name and its version. Plugins can be registered but not loaded
 *		at startup. It speeds up the application launch time.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY PluginInfo : public QObject
{
	Q_OBJECT
public:
	PluginInfo(QObject *parent = 0);

	/** Copy constructor required for converting in QVariant. */
	PluginInfo(const PluginInfo &other);

	PluginInfo& operator=(const PluginInfo& other);

	/** Destructor required for converting in QVariant. */
	virtual ~PluginInfo() {}

	/** Sets the filename. */
	void setFileName(const QString &f);

	/** Sets the name. */
	void setPluginName(const QString &p);

	/** Sets the version. */
	void setVersion(const QString &v);

	/** Sets if the plugin has its own config page. */
	void setConfigPage(bool b);

	/** Sets if the plugin is enabled. */
	void setEnabled(bool b);

	/** Gets the filename. */
	inline const QString& fileName() const { return _fileName; }

	/** Gets the name. */
	inline const QString& pluginName() const { return _pluginName; }

	/** Gets the version. */
	inline const QString& version() const { return _version; }

	/** True if the plugin has its own config page. */
	inline bool isConfigurable() const { return _configurable; }

	/** True if the plugin is enabled. */
	inline bool isEnabled() const { return _active; }

private:
	QString _fileName;
	QString _pluginName;
	QString _version;
	bool _configurable;
	bool _active;
};

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator<<(QDataStream &out, const PluginInfo &p)
{
	out << p.fileName() << p.pluginName() << p.version() << p.isConfigurable() << p.isEnabled();
	return out;
}

/** Overloaded to be able to use with QVariant. */
inline QDataStream & operator>>(QDataStream &in, PluginInfo &p)
{
	QString f, n, v;
	in >> f >> n >> v;
	bool c, e;
	in >> c >> e;
	p.setFileName(f);
	p.setPluginName(n);
	p.setVersion(v);
	p.setConfigPage(c);
	p.setEnabled(e);
	return in;
}

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(PluginInfo)

#endif // PLUGININFO_H
