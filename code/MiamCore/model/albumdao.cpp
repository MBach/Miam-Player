#include "albumdao.h"

AlbumDAO::AlbumDAO(QObject *parent) :
	GenericDAO(parent)
{}

AlbumDAO::AlbumDAO(const AlbumDAO &remoteAlbum) :
	GenericDAO(remoteAlbum)
{
	_artist = remoteAlbum.artist();
	_disc = remoteAlbum.disc();
	_iconPath = remoteAlbum.iconPath();
	_length = remoteAlbum.length();
	_source = remoteAlbum.source();
	_title = remoteAlbum.title();
	_uri = remoteAlbum.uri();
	_year = remoteAlbum.year();
}

AlbumDAO::~AlbumDAO() {}

QString AlbumDAO::artist() const { return _artist; }
void AlbumDAO::setArtist(const QString &artist) { _artist = artist; }

QString AlbumDAO::disc() const { return _disc; }
void AlbumDAO::setDisc(const QString &disc) { _disc = disc; }

QString AlbumDAO::iconPath() const { return _iconPath; }
void AlbumDAO::setIconPath(const QString &iconPath) { _iconPath = iconPath; }

QString AlbumDAO::length() const { return _length; }
void AlbumDAO::setLength(const QString &length) { _length = length; }

QString AlbumDAO::source() const { return _source; }
void AlbumDAO::setSource(const QString &source) { _source = source; }

QString AlbumDAO::title() const { return _title; }
void AlbumDAO::setTitle(const QString &title) { _title = title; }

QString AlbumDAO::uri() const { return _uri; }
void AlbumDAO::setUri(const QString &uri) { _uri = uri; }

QString AlbumDAO::year() const { return _year; }
void AlbumDAO::setYear(const QString &year) { _year = year; }
