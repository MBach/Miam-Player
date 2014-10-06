#include "playlistdao.h"

PlaylistDAO::PlaylistDAO(QObject *parent)
	: GenericDAO(parent)
{}

PlaylistDAO::PlaylistDAO(const PlaylistDAO &remotePlaylist)
	: GenericDAO(remotePlaylist)
{
	_background = remotePlaylist.background();
	_iconPath = remotePlaylist.iconPath();
	_length = remotePlaylist.length();
	_title = remotePlaylist.title();
}

PlaylistDAO::~PlaylistDAO()
{}

QString PlaylistDAO::background() const { return _background; }
void PlaylistDAO::setBackground(const QString &background) { _background = background; }

QString PlaylistDAO::iconPath() const { return _iconPath; }
void PlaylistDAO::setIconPath(const QString &iconPath) { _iconPath = iconPath; }

QString PlaylistDAO::length() const { return _length; }
void PlaylistDAO::setLength(const QString &length) { _length = length; }

QString PlaylistDAO::title() const { return _title; }
void PlaylistDAO::setTitle(const QString &title) { _title = title; }
