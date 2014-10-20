#include "artistitem.h"

ArtistItem::ArtistItem(const ArtistDAO *dao)
	: QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
}

int ArtistItem::type() const
{
	return LibraryTreeView::IT_Artist;
}
