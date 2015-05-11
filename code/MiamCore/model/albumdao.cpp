#include "albumdao.h"

AlbumDAO::AlbumDAO(QObject *parent) :
	GenericDAO(Miam::IT_Album, parent)
{}

AlbumDAO::AlbumDAO(const AlbumDAO &remoteAlbum) :
	GenericDAO(remoteAlbum),
	_artist(remoteAlbum.artist()),
	_disc(remoteAlbum.disc()),
	_cover(remoteAlbum.cover()),
	_length(remoteAlbum.length()),
	_source(remoteAlbum.source()),
	_uri(remoteAlbum.uri()),
	_year(remoteAlbum.year())
{}

AlbumDAO& AlbumDAO::operator=(const AlbumDAO& other)
{
	GenericDAO::operator=(other);
	_artist = other.artist();
	_disc = other.disc();
	_cover = other.cover();
	_length = other.length();
	_source = other.source();
	_uri = other.uri();
	_year = other.year();
	return *this;
}

AlbumDAO::~AlbumDAO() {}

QString AlbumDAO::artist() const { return _artist; }
void AlbumDAO::setArtist(const QString &artist) { _artist = artist; }

QString AlbumDAO::disc() const { return _disc; }
void AlbumDAO::setDisc(const QString &disc) { _disc = disc; }

QString AlbumDAO::cover() const { return _cover; }
void AlbumDAO::setCover(const QString &cover) { _cover = cover; }

QString AlbumDAO::length() const { return _length; }
void AlbumDAO::setLength(const QString &length) { _length = length; }

QString AlbumDAO::source() const { return _source; }
void AlbumDAO::setSource(const QString &source) { _source = source; }

QString AlbumDAO::uri() const { return _uri; }
void AlbumDAO::setUri(const QString &uri) { _uri = uri; }

QString AlbumDAO::year() const { return _year; }
void AlbumDAO::setYear(const QString &year) { _year = year; }

bool operator==(const AlbumDAO &d1, const AlbumDAO &d2)
{
	return  d1.checksum() == d2.checksum() &&
			d1.host() == d2.host() &&
			d1.icon() == d2.icon() &&
			d1.id() == d2.id() &&
			d1.title() == d2.title() &&
			d1.titleNormalized() == d2.titleNormalized() &&
			d1.parent() == d2.parent() &&
			d1.type() == d2.type() &&
			d1.artist() == d2.artist() &&
			d1.disc() == d2.disc() &&
			d1.cover() == d2.cover() &&
			d1.length() == d2.length() &&
			d1.source() == d2.source() &&
			d1.uri() == d2.uri() &&
			d1.year() == d2.year();
}

/*uint qHash(AlbumDAO *key)
{
	GenericDAO *parent = key;
	return qHash(parent) ^ qHash(key->title());
}*/
