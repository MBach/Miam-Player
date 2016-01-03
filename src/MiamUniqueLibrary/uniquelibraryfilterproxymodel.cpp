#include "uniquelibraryfilterproxymodel.h"

#include <QStandardItemModel>

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
		//break;
		return MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
	case Miam::IT_Album:
		return MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
		break;
	case Miam::IT_Track:
		/*for (QModelIndex index : _topLevelItems.values(static_cast<SeparatorItem*>(item))) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				return true;
			}

		}*/
		break;
	}
	return true;
}
