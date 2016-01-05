#include "uniquelibraryfilterproxymodel.h"

#include <QStandardItemModel>
#include <separatoritem.h>
#include <model/sqldatabase.h>
#include <QSqlQuery>

#include <QtDebug>

UniqueLibraryFilterProxyModel::UniqueLibraryFilterProxyModel(QObject *parent)
	: MiamSortFilterProxyModel(parent)
{

}

bool UniqueLibraryFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(sourceModel());
	QStandardItem *item = model->itemFromIndex(model->index(sourceRow, 0, sourceParent));
	if (!item) {
		return false;
	}
	bool result = false;
	switch (item->type()) {
	case Miam::IT_Artist:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else {
			QSqlQuery getArtist(*SqlDatabase::instance());
			getArtist.prepare("SELECT * FROM tracks WHERE title LIKE ? AND artistId = ?");
			getArtist.addBindValue("%" + filterRegExp().pattern() + "%");
			getArtist.addBindValue(item->data(Miam::DF_ID).toUInt());
			result = getArtist.exec() && getArtist.next();
		}
		break;
	case Miam::IT_Album:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else if (filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1) {
			result = true;
		} else {
			QSqlQuery getAlbum(*SqlDatabase::instance());
			getAlbum.prepare("SELECT * FROM tracks WHERE title LIKE ? AND albumId = ?");
			getAlbum.addBindValue("%" + filterRegExp().pattern() + "%");
			getAlbum.addBindValue(item->data(Miam::DF_ID).toUInt());
			result = getAlbum.exec() && getAlbum.next();
		}
		break;
	case Miam::IT_Track:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else {
			result = filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1 ||
					filterRegExp().indexIn(item->data(Miam::DF_Album).toString()) != -1;
		}
		break;
	case Miam::IT_Separator:
		for (QModelIndex index : _topLevelItems.values(static_cast<SeparatorItem*>(item))) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				result = true;
			}
		}
		break;
	default:
		break;
	}
	return result;
}
