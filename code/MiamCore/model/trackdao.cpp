#include "trackdao.h"

TrackDAO::TrackDAO(QObject *parent) :
	GenericDAO(parent), _rating(0)
{}

TrackDAO::TrackDAO(const TrackDAO &remoteTrack) :
	GenericDAO(remoteTrack)
{
	_album = remoteTrack.album();
	_artist = remoteTrack.artist();
	_artistAlbum = remoteTrack.artistAlbum();
	_disc = remoteTrack.disc();
	_iconPath = remoteTrack.iconPath();
	_length = remoteTrack.length();
	_rating = remoteTrack.rating();
	_source = remoteTrack.source();
	_title = remoteTrack.title();
	_trackNumber = remoteTrack.trackNumber();
	_url = remoteTrack.url();
	_year = remoteTrack.year();
}

TrackDAO::~TrackDAO() {}

QString TrackDAO::album() const { return _album; }
void TrackDAO::setAlbum(const QString &album) { _album = album; }

QString TrackDAO::artist() const { return _artist; }
void TrackDAO::setArtist(const QString &artist) { _artist = artist; }

QString TrackDAO::artistAlbum() const { return _artistAlbum; }
void TrackDAO::setArtistAlbum(const QString &artistAlbum) { _artistAlbum = artistAlbum; }

QString TrackDAO::disc() const { return _disc; }
void TrackDAO::setDisc(const QString &disc) { _disc = disc; }

QString TrackDAO::iconPath() const { return _iconPath; }
void TrackDAO::setIconPath(const QString &iconPath) { _iconPath = iconPath; }

QString TrackDAO::length() const { return _length; }
void TrackDAO::setLength(const QString &length) { _length = length; }

int TrackDAO::rating() const { return _rating; }
void TrackDAO::setRating(int rating) { _rating = rating; }

QString TrackDAO::source() const { return _source; }
void TrackDAO::setSource(const QString &source) { _source = source; }

QString TrackDAO::title() const { return _title; }
void TrackDAO::setTitle(const QString &title) { _title = title; }

QString TrackDAO::trackNumber() const { return _trackNumber; }
void TrackDAO::setTrackNumber(const QString &trackNumber) { _trackNumber = trackNumber; }

QString TrackDAO::url() const { return _url; }
void TrackDAO::setUrl(const QString &url) { _url = url; }

QString TrackDAO::year() const { return _year; }
void TrackDAO::setYear(const QString &year) { _year = year; }
