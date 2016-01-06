#ifndef UNIQUELIBRARYFILTERPROXYMODEL_H
#define UNIQUELIBRARYFILTERPROXYMODEL_H

#include "miamsortfilterproxymodel.h"
#include "miamuniquelibrary_global.hpp"

#include <QStandardItemModel>

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryFilterProxyModel : public MiamSortFilterProxyModel
{
	Q_OBJECT
private:
	QStandardItemModel *_model;

public:
	UniqueLibraryFilterProxyModel(QObject *parent = nullptr);

	/** Redefined from QSortFilterProxyModel. */
	void setSourceModel(QAbstractItemModel *sourceModel) override;

protected:
	/** Redefined from MiamSortFilterProxyModel. */
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // UNIQUELIBRARYFILTERPROXYMODEL_H
