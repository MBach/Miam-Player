#include "albumitem.h"

#include <QtDebug>

AlbumItem::AlbumItem(const AlbumDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), Miam::DF_NormalizedString);
	setData(dao->year(), Miam::DF_Year);
	setData(dao->cover(), Miam::DF_CoverPath);
	setData(dao->icon(), Miam::DF_IconPath);
	setData(!dao->icon().isEmpty(), Miam::DF_IsRemote);
}

QString AlbumItem::coverPath() const
{
	return data(Miam::DF_CoverPath).toString();
}

QString AlbumItem::iconPath() const
{
	return data(Miam::DF_IconPath).toString();
}

int AlbumItem::type() const
{
	return Miam::IT_Album;
}
