#include "artistdao.h"

ArtistDAO::ArtistDAO(QObject *parent) :
	GenericDAO(Miam::IT_Artist, parent)
{}

ArtistDAO::ArtistDAO(const ArtistDAO &remoteArtist) :
	GenericDAO(remoteArtist)
{
	_customData = remoteArtist.customData();
}

void ArtistDAO::setCustomData(const QString &data)
{
	_customData = data;
}

QString ArtistDAO::customData() const
{
	return _customData;
}

ArtistDAO::~ArtistDAO() {}

#include <QHash>

uint ArtistDAO::hash() const
{
	if (parentNode()) {
		return GenericDAO::hash() ^ parentNode()->hash();
	} else {
		return GenericDAO::hash();
	}
}
