#ifndef PLUGININFO_H
#define PLUGININFO_H

#include <QDataStream>
#include <QObject>

class PluginInfo : public QObject
{
	Q_OBJECT

public:
	PluginInfo(QObject *parent = 0);

	PluginInfo(const PluginInfo &pluginInfo);

	virtual ~PluginInfo() {}

	void setFileName(const QString &f);
	void setPluginName(const QString &p);
	void setVersion(const QString &v);
	void setConfigPage(bool b);
	void setEnabled(bool b);

	inline const QString& fileName() const { return _fileName; }
	inline const QString& pluginName() const { return _pluginName; }
	inline const QString& version() const { return _version; }
	inline bool configPage() const { return _configPage; }
	inline bool isEnabled() const { return _active; }

private:
	QString _fileName;
	QString _pluginName;
	QString _version;
	bool _configPage;
	bool _active;
};

inline QDataStream & operator<<(QDataStream &out, const PluginInfo &p)
{
	out << p.fileName() << p.pluginName() << p.version() << p.configPage() << p.isEnabled();
	return out;
}

inline QDataStream & operator>>(QDataStream &in, PluginInfo &p)
{
	QString f, n, v;
	in >> f >> n >> v;
	p.setFileName(f);
	p.setPluginName(n);
	p.setVersion(v);
	bool c, e;
	in >> c >> e;
	p.setConfigPage(c);
	p.setEnabled(e);
	return in;
}

Q_DECLARE_METATYPE(PluginInfo)

#endif // PLUGININFO_H
