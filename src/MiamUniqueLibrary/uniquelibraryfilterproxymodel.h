#ifndef UNIQUELIBRARYFILTERPROXYMODEL_H
#define UNIQUELIBRARYFILTERPROXYMODEL_H

#include "miamsortfilterproxymodel.h"
#include "miamuniquelibrary_global.hpp"

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryFilterProxyModel : public MiamSortFilterProxyModel
{
	Q_OBJECT
public:
	UniqueLibraryFilterProxyModel(QObject *parent = nullptr);

protected:
	/** Redefined from QSortFilterProxyModel. */
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // UNIQUELIBRARYFILTERPROXYMODEL_H
