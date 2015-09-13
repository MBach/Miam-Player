#ifndef UNIQUELIBRARYITEMMODEL_H
#define UNIQUELIBRARYITEMMODEL_H

#include <miamitemmodel.h>
#include "miamuniquelibrary_global.h"

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryItemModel : public MiamItemModel
{
	Q_OBJECT
private:
	QSortFilterProxyModel *_proxy;

public:
	explicit UniqueLibraryItemModel(QObject *parent = 0);

	virtual QSortFilterProxyModel* proxy() const override;

public slots:
	virtual void insertNode(GenericDAO *node) override;
};

#endif // UNIQUELIBRARYITEMMODEL_H
