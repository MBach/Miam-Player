#ifndef UNIQUELIBRARYFILTERPROXYMODEL_H
#define UNIQUELIBRARYFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <separatoritem.h>
#include "miamuniquelibrary_global.h"

/**
 * \brief		The UniqueLibraryFilterProxyModel class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
private:
	/** Top levels items are specific items, like letters 'A', 'B', ... in the library. Each letter has a reference to all items beginning with this letter. */
	QMultiHash<SeparatorItem*, QModelIndex> _topLevelItems;

public:
	UniqueLibraryFilterProxyModel(QObject *parent = 0);

protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

	/** Redefined for custom sorting. */
	virtual bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const override;

private:
	bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;

	bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // UNIQUELIBRARYFILTERPROXYMODEL_H
