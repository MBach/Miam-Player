#include "genericdao.h"

GenericDAO::GenericDAO(QObject *parent) :
	QObject(parent)
{}

GenericDAO::GenericDAO(const GenericDAO &remoteObject) :
	QObject(remoteObject.parent())
{
	_checksum = remoteObject.checksum();
	_host = remoteObject.host();
	_id = remoteObject.id();
}

GenericDAO::~GenericDAO()
{}

QString GenericDAO::checksum() const { return _checksum; }
void GenericDAO::setChecksum(const QString &checksum) { _checksum = checksum; }

QString GenericDAO::host() const { return _host; }
void GenericDAO::setHost(const QString &host) { _host = host; }

QString GenericDAO::id() const { return _id; }
void GenericDAO::setId(const QString &id) { _id = id; }
