#ifndef UNIQUELIBRARYITEMDELEGATE_H
#define UNIQUELIBRARYITEMDELEGATE_H

#include <miamitemdelegate.h>
#include "miamuniquelibrary_global.h"

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryItemDelegate : public MiamItemDelegate
{
	Q_OBJECT
public:
	UniqueLibraryItemDelegate(LibraryFilterProxyModel *proxy);

	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // UNIQUELIBRARYITEMDELEGATE_H
