#include "albumitem.h"

#include <QtDebug>

AlbumItem::AlbumItem(const AlbumDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->year(), LibraryTreeView::DF_Year);
	setData(dao->cover(), LibraryTreeView::DF_CoverPath);
	setData(dao->icon(), LibraryTreeView::DF_IconPath);
	setData(!dao->icon().isEmpty(), LibraryTreeView::DF_IsRemote);
}

QString AlbumItem::coverPath() const
{
	return data(LibraryTreeView::DF_CoverPath).toString();
}

QString AlbumItem::iconPath() const
{
	return data(LibraryTreeView::DF_IconPath).toString();
}

int AlbumItem::type() const
{
	return LibraryTreeView::IT_Album;
}
