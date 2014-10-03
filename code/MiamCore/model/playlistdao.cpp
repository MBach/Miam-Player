#include "playlistdao.h"

PlaylistDAO::PlaylistDAO(QObject *parent)
	: GenericDAO(parent)
{}

PlaylistDAO::PlaylistDAO(const PlaylistDAO &remotePlaylist)
	: GenericDAO(remotePlaylist)
{
	_icon = remotePlaylist.icon();
	_length = remotePlaylist.length();
	_title = remotePlaylist.title();
}

PlaylistDAO::~PlaylistDAO()
{}

QIcon PlaylistDAO::icon() const { return _icon; }
void PlaylistDAO::setIcon(const QIcon &icon) { _icon = icon; }

QString PlaylistDAO::length() const { return _length; }
void PlaylistDAO::setLength(const QString &length) { _length = length; }

QString PlaylistDAO::title() const { return _title; }
void PlaylistDAO::setTitle(const QString &title) { _title = title; }
