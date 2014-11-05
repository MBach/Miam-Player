#include "playlistdao.h"

PlaylistDAO::PlaylistDAO(QObject *parent)
	: GenericDAO(parent)
{}

PlaylistDAO::PlaylistDAO(const PlaylistDAO &other)
	: GenericDAO(other)
{
	_background = other.background();
	_length = other.length();
}

PlaylistDAO::~PlaylistDAO()
{}

QString PlaylistDAO::background() const { return _background; }
void PlaylistDAO::setBackground(const QString &background) { _background = background; }

QString PlaylistDAO::length() const { return _length; }
void PlaylistDAO::setLength(const QString &length) { _length = length; }
