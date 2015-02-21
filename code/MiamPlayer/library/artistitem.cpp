#include "artistitem.h"

ArtistItem::ArtistItem(const ArtistDAO *dao)
	: QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryFilterProxyModel::DF_NormalizedString);
}

int ArtistItem::type() const
{
	return LibraryFilterProxyModel::IT_Artist;
}
