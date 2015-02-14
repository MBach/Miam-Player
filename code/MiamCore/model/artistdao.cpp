#include "artistdao.h"

ArtistDAO::ArtistDAO(QObject *parent) :
	GenericDAO(parent, GenericDAO::Artist)
{}

ArtistDAO::ArtistDAO(const ArtistDAO &remoteArtist) :
	GenericDAO(remoteArtist)
{
}

ArtistDAO::~ArtistDAO() {}
