#include "artistitem.h"
#include "miamcore_global.h"

ArtistItem::ArtistItem()
{}

int ArtistItem::type() const
{
	return Miam::IT_Artist;
}

uint ArtistItem::hash() const
{
	return qHash(text()) * qHash(data(Miam::DF_NormalizedString).toString()) * qHash(type());
}
