#include "remotetrack.h"

RemoteTrack::RemoteTrack(QObject *parent) :
	QObject(parent)
{}

RemoteTrack::RemoteTrack(const RemoteTrack &remoteTrack) :
	QObject(remoteTrack.parent())
{
	_album = remoteTrack.album();
	_artist = remoteTrack.artist();
	_icon = remoteTrack.icon();
	_id = remoteTrack.id();
	_length = remoteTrack.length();
	_rating = remoteTrack.rating();
	_title = remoteTrack.title();
	_trackNumber = remoteTrack.trackNumber();
	_url = remoteTrack.url();
	_year = remoteTrack.year();
}

RemoteTrack::~RemoteTrack() {}

QString RemoteTrack::album() const { return _album; }
void RemoteTrack::setAlbum(const QString &album) { _album = album; }

QString RemoteTrack::artist() const { return _artist; }
void RemoteTrack::setArtist(const QString &artist) { _artist = artist; }

QString RemoteTrack::disc() const { return _disc; }
void RemoteTrack::setDisc(const QString &disc) { _disc = disc; }

QIcon RemoteTrack::icon() const { return _icon; }
void RemoteTrack::setIcon(const QIcon &icon) { _icon = icon; }

QString RemoteTrack::id() const { return _id; }
void RemoteTrack::setId(const QString &id) { _id = id; }

QString RemoteTrack::length() const { return _length; }
void RemoteTrack::setLength(const QString &length) { _length = length; }

int RemoteTrack::rating() const { return _rating; }
void RemoteTrack::setRating(int rating) { _rating = rating; }

QString RemoteTrack::title() const { return _title; }
void RemoteTrack::setTitle(const QString &title) { _title = title; }

QString RemoteTrack::trackNumber() const { return _trackNumber; }
void RemoteTrack::setTrackNumber(const QString &trackNumber) { _trackNumber = trackNumber; }

QString RemoteTrack::url() const { return _url; }
void RemoteTrack::setUrl(const QString &url) { _url = url; }

QString RemoteTrack::year() const { return _year; }
void RemoteTrack::setYear(const QString &year) { _year = year; }
