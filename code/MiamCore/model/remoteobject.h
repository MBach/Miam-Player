#ifndef REMOTEOBJECT_H
#define REMOTEOBJECT_H

#include <QObject>
#include "../miamcore_global.h"

/**
 * \brief		The RemoteObject class is a simple wrapper which contains basic informations about a file.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY RemoteObject : public QObject
{
	Q_OBJECT
private:
	QString _checksum, _id;
public:
	explicit RemoteObject(QObject *parent = 0);

	RemoteObject(const RemoteObject &remoteObject);

	virtual ~RemoteObject();

	QString checksum() const;
	void setChecksum(const QString &checksum);

	QString id() const;
	void setId(const QString &id);
};

#endif // REMOTEOBJECT_H
