#include "remoteobject.h"

RemoteObject::RemoteObject(QObject *parent) :
	QObject(parent)
{}

RemoteObject::RemoteObject(const RemoteObject &remoteObject) :
	QObject(remoteObject.parent())
{
	_checksum = remoteObject.checksum();
	_host = remoteObject.host();
	_id = remoteObject.id();
}

RemoteObject::~RemoteObject()
{}

QString RemoteObject::checksum() const { return _checksum; }
void RemoteObject::setChecksum(const QString &checksum) { _checksum = checksum; }

QString RemoteObject::host() const { return _host; }
void RemoteObject::setHost(const QString &host) { _host = host; }

QString RemoteObject::id() const { return _id; }
void RemoteObject::setId(const QString &id) { _id = id; }
