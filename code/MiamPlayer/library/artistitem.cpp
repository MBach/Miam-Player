#include "artistitem.h"

ArtistItem::ArtistItem(const QString &text)
	: QStandardItem(text)
{}

int ArtistItem::type() const
{
	return LibraryTreeView::IT_Artist;
}
