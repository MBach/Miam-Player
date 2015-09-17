#ifndef UNIQUELIBRARYITEMDELEGATE_H
#define UNIQUELIBRARYITEMDELEGATE_H

#include <miamitemdelegate.h>
#include <library/jumptowidget.h>
#include "miamuniquelibrary_global.h"

#include <trackitem.h>

/**
 * \brief		The UniqueLibraryItemDelegate class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryItemDelegate : public MiamItemDelegate
{
	Q_OBJECT
private:
	JumpToWidget *_jumpTo;

public:
	UniqueLibraryItemDelegate(JumpToWidget *jumpTo, QSortFilterProxyModel *proxy);

	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // UNIQUELIBRARYITEMDELEGATE_H
