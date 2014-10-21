#include "albumitem.h"

#include <QtDebug>

AlbumItem::AlbumItem(const AlbumDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->year(), LibraryTreeView::DF_Year);
	setData(dao->cover(), LibraryTreeView::DF_CoverPath);
	setData(!dao->icon().isEmpty(), LibraryTreeView::DF_IsRemote);
	setData(QVariant::fromValue(*dao), LibraryTreeView::DF_DAO);
}

int AlbumItem::type() const
{
	return LibraryTreeView::IT_Album;
}
