#include "remoteplaylist.h"

RemotePlaylist::RemotePlaylist(QObject *parent)
	: RemoteObject(parent)
{}

RemotePlaylist::RemotePlaylist(const RemotePlaylist &remotePlaylist)
	: RemoteObject(remotePlaylist)
{
	_length = remotePlaylist.length();
	_title = remotePlaylist.title();
}

RemotePlaylist::~RemotePlaylist()
{}

QString RemotePlaylist::length() const { return _length; }
void RemotePlaylist::setLength(const QString &length) { _length = length; }

QString RemotePlaylist::title() const { return _title; }
void RemotePlaylist::setTitle(const QString &title) { _title = title; }
