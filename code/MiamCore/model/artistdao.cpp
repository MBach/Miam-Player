#include "artistdao.h"

ArtistDAO::ArtistDAO(QObject *parent) :
	GenericDAO(parent)
{}

ArtistDAO::ArtistDAO(const ArtistDAO &remoteArtist) :
	GenericDAO(remoteArtist)
{
}

ArtistDAO::~ArtistDAO() {}
