#include "albumitem.h"

#include <QtDebug>

AlbumItem::AlbumItem(const AlbumDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryFilterProxyModel::DF_NormalizedString);
	setData(dao->year(), LibraryFilterProxyModel::DF_Year);
	setData(dao->cover(), LibraryFilterProxyModel::DF_CoverPath);
	setData(dao->icon(), LibraryFilterProxyModel::DF_IconPath);
	setData(!dao->icon().isEmpty(), LibraryFilterProxyModel::DF_IsRemote);
}

QString AlbumItem::coverPath() const
{
	return data(LibraryFilterProxyModel::DF_CoverPath).toString();
}

QString AlbumItem::iconPath() const
{
	return data(LibraryFilterProxyModel::DF_IconPath).toString();
}

int AlbumItem::type() const
{
	return LibraryFilterProxyModel::IT_Album;
}
