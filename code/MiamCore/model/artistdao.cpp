#include "artistdao.h"

ArtistDAO::ArtistDAO(QObject *parent) :
	GenericDAO(parent, GenericDAO::Artist)
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
