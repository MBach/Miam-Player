#include "libraryfilterproxymodel.h"

#include "settings.h"

#include <QtDebug>

#include <QPainter>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	//this->setSortRole(LibraryItem::NormalizedString);
	this->sort(0, Qt::DescendingOrder);
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
	return false;
}

#include "librarytreeview.h"

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
		rType = right->data(LibraryTreeView::Type).toInt();
		if (rType == LibraryTreeView::Album) {
			int lYear = left->data(LibraryTreeView::Year).toInt();
			int rYear = right->data(LibraryTreeView::Year).toInt();
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
		}
		break;

	case LibraryTreeView::Disc:
		if (rType == LibraryTreeView::Disc) {
			int dLeft = left->data(LibraryTreeView::DataDiscNumber).toInt();
			int dRight = right->data(LibraryTreeView::DataDiscNumber).toInt();
			result = dLeft < dRight;
		} /*else if (model->itemFromIndex(idxRight)->data(LibraryTreeView::Type).toInt() == LibraryTreeView::Track) {
			int dLeft = leftDisc->data(LibraryTreeView::DataDiscNumber).toInt();
			int dRight = rightTrack->data(LibraryTreeView::DataDiscNumber).toInt();
			result = dLeft < dRight;
		}*/
		break;

	case LibraryTreeView::Letter:
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)

		if (right->data(LibraryTreeView::Type).toInt() == LibraryTreeView::Album) {
			qDebug() << left->text() << right->data(LibraryTreeView::DataNormalizedString).toString().left(1) << "(" << right->data(LibraryTreeView::DataNormalizedString).toString() << ")";
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
		/*if (right) {
			int dLeft = left->data(LibraryTreeView::DataDiscNumber).toInt();
			int dRight = right->data(LibraryTreeView::DataDiscNumber).toInt();
			if (dLeft == dRight) {
				result = (leftTrack->trackNumber() < rightTrack->trackNumber() && sortOrder() == Qt::AscendingOrder) ||
						(leftTrack->trackNumber() > rightTrack->trackNumber() && sortOrder() == Qt::DescendingOrder);
			} else {
				result = (leftTrack->discNumber() < rightTrack->discNumber() && sortOrder() == Qt::AscendingOrder) ||
						  (leftTrack->discNumber() > rightTrack->discNumber() && sortOrder() == Qt::DescendingOrder);
			}
		} else {*/
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		//}
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
