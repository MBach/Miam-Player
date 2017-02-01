#include "mbrelease.h"

using namespace MusicBrainz;

/** Default copy constructor. */
Artist::Artist(const Artist &other)
	: QObject(other.parent())
{
	id = other.id;
	name = other.name;
}

Artist& Artist::operator=(const Artist& other)
{
	id = other.id;
	name = other.name;
	return *this;
}


/** Default copy constructor. */
Track::Track(const Track &other)
	: QObject(other.parent())
{
	id = other.id;
	title = other.title;
	position = other.position;
	length = other.length;
	artist = new Artist(this);
	artist->id = other.artist->id;
	artist->name = other.artist->name;
}

Track& Track::operator=(const Track& other)
{
	id = other.id;
	title = other.title;
	position = other.position;
	length = other.length;
	artist = new Artist;
	artist->id = other.artist->id;
	artist->name = other.artist->name;
	return *this;
}


Release::Release(const Release &other)
	: QObject(other.parent())
{
	id = other.id;
	releaseGroupId = other.releaseGroupId;
	trackCount = other.trackCount;
	title = other.title;
	country = other.country;
	year = other.year;
	format = other.format;
	disc = other.disc;
	tracks = other.tracks;
	artist = other.artist;
}

Release& Release::operator=(const Release& other)
{
	id = other.id;
	releaseGroupId = other.releaseGroupId;
	trackCount  = other.trackCount;
	title = other.title;
	country = other.country;
	year = other.year;
	format = other.format;
	disc = other.disc;
	tracks = other.tracks;
	artist = other.artist;
	return *this;
}

Track Release::track(const QString &filename) const
{
	return tracks.value(filename);
}
