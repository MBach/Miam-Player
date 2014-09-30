#include "remoteobject.h"

RemoteObject::RemoteObject(QObject *parent) :
	QObject(parent)
{}

RemoteObject::RemoteObject(const RemoteObject &remoteObject) :
	QObject(remoteObject.parent())
{
	_checksum = remoteObject.checksum();
	_id = remoteObject.id();
}

RemoteObject::~RemoteObject()
{}

QString RemoteObject::checksum() const { return _checksum; }
void RemoteObject::setChecksum(const QString &checksum) { _checksum = checksum; }

QString RemoteObject::id() const { return _id; }
void RemoteObject::setId(const QString &id) { _id = id; }
