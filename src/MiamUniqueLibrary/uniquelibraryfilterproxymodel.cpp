#include "uniquelibraryfilterproxymodel.h"

#include <QStandardItemModel>
#include <separatoritem.h>

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
	switch (item->type()) {
	case Miam::IT_Artist:
		return MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
	case Miam::IT_Album:
		return MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) ||
				filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1;
	case Miam::IT_Track:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			// Accept parent Separators, Artists & Album (and Disc if present)
			//qDebug() << Q_FUNC_INFO << item->text() << sourceRow << sourceParent;
			//MiamSortFilterProxyModel::filterAcceptsRow(item->data(Miam::DF_ArtistID).toInt(), sourceParent);
			//MiamSortFilterProxyModel::filterAcceptsRow(albumRow, sourceParent);
			return true;
		} else {
			return filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1 ||
					filterRegExp().indexIn(item->data(Miam::DF_Album).toString()) != -1;
		}
	case Miam::IT_Separator:
		for (QModelIndex index : _topLevelItems.values(static_cast<SeparatorItem*>(item))) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				return true;
			}
		}
	default:
		break;
	}
	return false;
}
