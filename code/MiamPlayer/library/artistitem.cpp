#include "artistitem.h"

ArtistItem::ArtistItem(const ArtistDAO *dao)
	: QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), Miam::DF_NormalizedString);
}

int ArtistItem::type() const
{
	return Miam::IT_Artist;
}
