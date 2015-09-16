#include "uniquelibraryfilterproxymodel.h"

#include <miamcore_global.h>
#include <settingsprivate.h>
#include <QStandardItem>

#include <albumitem.h>

#include <QtDebug>

UniqueLibraryFilterProxyModel::UniqueLibraryFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(Miam::DF_NormalizedString);
	this->setDynamicSortFilter(false);
	this->sort(0, Qt::AscendingOrder);}

bool UniqueLibraryFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->sourceModel());
	QStandardItem *left = model->itemFromIndex(idxLeft);
	//QStandardItem *right = model->itemFromIndex(idxRight);

	int lType = left->type();
	//int rType = right->type();
	switch (lType) {
	default:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}
	return result;
}
