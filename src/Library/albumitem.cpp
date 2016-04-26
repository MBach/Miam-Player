#include "albumitem.h"
#include "miamcore_global.h"

AlbumItem::AlbumItem()
{}

int AlbumItem::type() const
{
	return Miam::IT_Album;
}

uint AlbumItem::hash() const
{
	uint h1 = qHash(data(Miam::DF_NormalizedString).toString(), 0x9e3779b9);
	uint h2;
	if (data(Miam::DF_Year).toString().isEmpty()) {
		h2 = 0x9e3779b9;
	} else {
		h2 = qHash(data(Miam::DF_Year).toString(), 0x9e3779b9);
	}
	uint h3 = qHash(data(Miam::DF_NormArtist).toString(), 0x9e3779b9);
	uint h4 = qHash(type(), 0x9e3779b9);
	return h1 * h2 * h3 * h4;
}
