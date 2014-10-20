#include "albumitem.h"

AlbumItem::AlbumItem(const AlbumDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->year(), LibraryTreeView::DF_Year);
	setData(dao->cover(), LibraryTreeView::DF_CoverPath);
}

int AlbumItem::type() const
{
	return LibraryTreeView::IT_Album;
}
