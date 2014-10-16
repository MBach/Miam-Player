#ifndef LIBRARYFILTERPROXYMODEL_H
#define LIBRARYFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QStandardItem>

/**
 * \brief		The LibraryFilterProxyModel class is used to filter Library by looking in all items
 * \details		When filtering, the method filterAcceptsRow will not stop if a search term was not found in a node. The algorithm
 *				will continue recursively until all subnodes and leaves are evaluated.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class LibraryFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
private:
	/** Top levels items are specific items, like letters 'A', 'B', ... in the library. Each letter has a reference to all items beginning with this letter. */
	QMultiHash<QModelIndex, QModelIndex> *_topLevelItems;

	// QList<QList<QModelIndex>> _items;

public:
	explicit LibraryFilterProxyModel(QObject *parent = 0);

	inline void setTopLevelItems(QMultiHash<QModelIndex, QModelIndex> *topLevelItems) { _topLevelItems = topLevelItems; }

	/** Redefined to override Qt::FontRole. */
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	QStandardItem* find(int level, const QString &parent) const;

protected:
	/** Redefined from QSortFilterProxyModel. */
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &parent) const;

	/** Redefined for custom sorting. */
	virtual bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const;

private:
	bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;
	bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;

signals:
	void aboutToHighlight(const QModelIndex &, bool b) const;
};

#endif // LIBRARYFILTERPROXYMODEL_H
