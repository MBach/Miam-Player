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
	return qHash(text(), 0x9e3779b9) ^ qHash(data(Miam::DF_NormalizedString).toString(), 0x9e3779b9) ^ qHash(type(), 0x9e3779b9);
}
