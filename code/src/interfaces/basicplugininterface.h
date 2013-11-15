#ifndef BASICPLUGININTERFACE_H
#define BASICPLUGININTERFACE_H

#include <QString>
#include <QtPlugin>

class BasicPluginInterface : public QObject
{
	Q_OBJECT
public:
	virtual ~BasicPluginInterface() {}

	virtual QString name() const = 0;

	virtual QString version() const = 0;

	virtual QWidget* configPage() = 0;
};

#define BasicPluginInterface_iid "MmeMiamMiamMusicPlayer.BasicPluginInterface"

Q_DECLARE_INTERFACE(BasicPluginInterface, BasicPluginInterface_iid)

#endif // BASICPLUGININTERFACE_H
