#include "libraryfilterproxymodel.h"

#include "settings.h"
#include "librarytreeview.h"

#include <QtDebug>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent), _topLevelItems(NULL)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(LibraryTreeView::DataNormalizedString);
	this->sort(0, Qt::AscendingOrder);
}

QVariant LibraryFilterProxyModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::FontRole) {
		return Settings::getInstance()->font(Settings::LIBRARY);
	} else {
		return QSortFilterProxyModel::data(index, role);
	}
}

bool LibraryFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (filterAcceptsRowItself(sourceRow, sourceParent)) {
		if (!filterRegExp().isEmpty()) {
			emit aboutToExpand(mapFromSource(sourceParent));
		}
		return true;
	}

	// Accept if any of the parents is accepted on it's own merits
	QModelIndex parent = sourceParent;
	while (parent.isValid()) {
		if (filterAcceptsRowItself(parent.row(), parent.parent())) {
			return true;
		}
		parent = parent.parent();
	}

	// Accept if any of the children is accepted on it's own merits
	if (hasAcceptedChildren(sourceRow, sourceParent)) {
		return true;
	}

	// Accept separators if any top level items and its children are accepted
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(sourceModel());
	QStandardItem *item = model->itemFromIndex(model->index(sourceRow, 0, sourceParent));
	if (item && item->data(LibraryTreeView::Type).toInt() == LibraryTreeView::Letter) {
		foreach (QModelIndex index, _topLevelItems->values(item->index())) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				return true;
			}
		}
	}
	return false;
}

/** Redefined for custom sorting. */
bool LibraryFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->sourceModel());
	QStandardItem *left = model->itemFromIndex(idxLeft);
	QStandardItem *right = model->itemFromIndex(idxRight);

	int lType = left->data(LibraryTreeView::Type).toInt();
	int rType = right->data(LibraryTreeView::Type).toInt();
	switch (lType) {
	case LibraryTreeView::Artist:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;

	case LibraryTreeView::Album:
		if (rType == LibraryTreeView::Album) {
			int lYear = left->data(LibraryTreeView::DataYear).toInt();
			int rYear = right->data(LibraryTreeView::DataYear).toInt();
			if (Settings::getInstance()->value("insertPolicy").toInt() == LibraryTreeView::Artist && lYear >= 0 && rYear >= 0) {
				if (sortOrder() == Qt::AscendingOrder) {
					if (lYear == rYear) {
						result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
					} else {
						result = lYear < rYear;
					}
				} else {
					result = lYear > rYear;
				}
			} else {
				result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	case LibraryTreeView::Disc:
		if (rType == LibraryTreeView::Disc) {
			int dLeft = left->data(LibraryTreeView::DataDiscNumber).toInt();
			int dRight = right->data(LibraryTreeView::DataDiscNumber).toInt();
			result = dLeft < dRight;
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	case LibraryTreeView::Letter:
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)
		if (rType == LibraryTreeView::Artist || rType == LibraryTreeView::Album) {
			if (QString::compare(left->text().left(1), right->data(LibraryTreeView::DataNormalizedString).toString().left(1)) == 0) {
				result = (sortOrder() == Qt::AscendingOrder);
			} else {
				result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		} else {
			if (QString::compare(left->text().left(1), right->text().left(1)) == 0) {
				result = (sortOrder() == Qt::AscendingOrder);
			} else {
				result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		}
		break;

	// Sort tracks by their numbers
	case LibraryTreeView::Track:
		if (rType == LibraryTreeView::Track) {
			int dLeft = left->data(LibraryTreeView::DataDiscNumber).toInt();
			int dRight = right->data(LibraryTreeView::DataDiscNumber).toInt();
			int lTrackNumber = left->data(LibraryTreeView::DataTrackNumber).toInt();
			int rTrackNumber = right->data(LibraryTreeView::DataTrackNumber).toInt();
			if (dLeft == dRight) {
				result = (lTrackNumber < rTrackNumber && sortOrder() == Qt::AscendingOrder) ||
						(lTrackNumber > rTrackNumber && sortOrder() == Qt::DescendingOrder);
			} else {
				result = (dLeft < dRight && sortOrder() == Qt::AscendingOrder) ||
						  (dLeft > dRight && sortOrder() == Qt::DescendingOrder);
			}
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	default:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}
	return result;
}

bool LibraryFilterProxyModel::filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool LibraryFilterProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);
	if (!item.isValid()) {
		return false;
	}

	// Check if there are children
	int childCount = item.model()->rowCount(item);
	if (childCount == 0) {
		return false;
	}

	for (int i = 0; i < childCount; ++i) {
		if (filterAcceptsRowItself(i, item)) {
			return true;
		}
		// Recursive call
		if (hasAcceptedChildren(i, item)) {
			return true;
		}
	}
	return false;
}
