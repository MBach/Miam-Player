#ifndef UNIQUELIBRARYFILTERPROXYMODEL_H
#define UNIQUELIBRARYFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "miamuniquelibrary_global.h"

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryFilterProxyModel : public QSortFilterProxyModel
{
public:
	UniqueLibraryFilterProxyModel(QObject *parent = 0);

protected:
	/** Redefined for custom sorting. */
	virtual bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const override;
};

#endif // UNIQUELIBRARYFILTERPROXYMODEL_H
