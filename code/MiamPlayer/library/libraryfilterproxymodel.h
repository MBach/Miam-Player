#ifndef LIBRARYFILTERPROXYMODEL_H
#define LIBRARYFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "librarymodel.h"

class LibraryFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	LibraryFilterProxyModel(QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
	/** Redefined from QSortFilterProxyModel. */
	bool filterAcceptsRow(int sourceRow, const QModelIndex &parent) const;

	/** Redefined for custom sorting. */
	bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const;

private:
	bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;
	bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;

signals:
	void aboutToExpand(const QModelIndex &) const;
};

#endif // LIBRARYFILTERPROXYMODEL_H
