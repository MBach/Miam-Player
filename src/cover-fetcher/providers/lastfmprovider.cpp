#include "lastfmprovider.h"

LastFMProvider::LastFMProvider(QNetworkAccessManager *parent)
	: CoverArtProvider(parent)
{

}

QUrl LastFMProvider::query(const QString &artist, const QString &album)
{
	return QUrl("http://ws.audioscrobbler.com/2.0/?artist.search=" + artist + "&artist.search=" + album);
}

QUrl LastFMProvider::album(const QString &expr)
{
	return QUrl("http://ws.audioscrobbler.com/2.0/?album.getInfo=" + expr);
}
