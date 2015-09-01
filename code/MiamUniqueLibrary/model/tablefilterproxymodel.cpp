#include "tablefilterproxymodel.h"

#include <QStandardItemModel>

#include "miamcore_global.h"

#include <QtDebug>

TableFilterProxyModel::TableFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(DF_NormalizedString);
	this->setDynamicSortFilter(false);
	this->sort(0, Qt::AscendingOrder);
}

/** Redefined for custom sorting. */
bool TableFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->sourceModel());
	QStandardItem *left = model->itemFromIndex(idxLeft);
	QStandardItem *right = model->itemFromIndex(idxRight);

	int lType = left->type();
	int rType = right->type();
	switch (lType) {
	case Miam::IT_Artist: {
		if (rType == Miam::IT_Artist) {
			qDebug() << Q_FUNC_INFO << "Miam::IT_Artist" << left->text() << right->text();
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		} else {
			result = false;
		}
		break;
	}
	default: {
		//qDebug() << Q_FUNC_INFO << "default";
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}
	}
	return result;
}
