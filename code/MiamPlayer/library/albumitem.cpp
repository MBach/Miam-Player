#include "albumitem.h"

AlbumItem::AlbumItem(const QString &text) :
	QStandardItem(text)
{}

int AlbumItem::type() const
{
	return LibraryTreeView::IT_Album;
}
