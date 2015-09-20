#ifndef LIBRARYFILTERPROXYMODEL_H
#define LIBRARYFILTERPROXYMODEL_H

#include <QStandardItem>
#include <miamsortfilterproxymodel.h>

#include "miamcore_global.h"
#include "separatoritem.h"
#include "miamlibrary_global.h"

/**
 * \brief		The LibraryFilterProxyModel class is used to filter Library by looking in all items
 * \details		When filtering, the method filterAcceptsRow will not stop if a search term was not found in a node. The algorithm
 *				will continue recursively until all subnodes and leaves are evaluated.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryFilterProxyModel : public MiamSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit LibraryFilterProxyModel(QObject *parent = 0);

	/** Redefined to override Qt::FontRole. */
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
	/** Redefined from QSortFilterProxyModel. */
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &parent) const override;

	/** Redefined for custom sorting. */
	virtual bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const override;

private:
	bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;
	bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // LIBRARYFILTERPROXYMODEL_H
