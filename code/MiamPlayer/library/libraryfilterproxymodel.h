#ifndef LIBRARYFILTERPROXYMODEL_H
#define LIBRARYFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class LibraryFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

private:
	QMultiHash<QModelIndex, QModelIndex> *_topLevelItems;

public:
	LibraryFilterProxyModel(QObject *parent = 0);

	inline void setTopLevelItems(QMultiHash<QModelIndex, QModelIndex> *topLevelItems) { _topLevelItems = topLevelItems; }

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
