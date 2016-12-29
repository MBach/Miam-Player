#include "libraryfilterproxymodel.h"

#include <settingsprivate.h>

#include <QtDebug>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	MiamSortFilterProxyModel(parent)
{}

/** Redefined to override Qt::FontRole. */
QVariant LibraryFilterProxyModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::FontRole) {
		return SettingsPrivate::instance()->font(SettingsPrivate::FF_Library);
	} else {
		return MiamSortFilterProxyModel::data(index, role);
	}
}

bool LibraryFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (filterAcceptsRowItself(sourceRow, sourceParent)) {
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
	if (item && item->type() == Miam::IT_Separator) {
		for (QModelIndex index : _topLevelItems.values(static_cast<SeparatorItem*>(item))) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				return true;
			}
		}
	}
	return (SettingsPrivate::instance()->librarySearchMode() == SettingsPrivate::LSM_HighlightOnly);
}

/** Redefined for custom sorting. */
bool LibraryFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->sourceModel());
	QStandardItem *left = model->itemFromIndex(idxLeft);
	QStandardItem *right = model->itemFromIndex(idxRight);

	int lType = left->type();
	int rType = right->type();
	switch (lType) {
	case Miam::IT_Artist:
		result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;

	case Miam::IT_Album:
		if (rType == Miam::IT_Album) {
			int lYear = left->data(Miam::DF_Year).toInt();
			int rYear = right->data(Miam::DF_Year).toInt();
			if (SettingsPrivate::instance()->insertPolicy() == SettingsPrivate::IP_Artists && lYear >= 0 && rYear >= 0) {
				if (sortOrder() == Qt::AscendingOrder) {
					if (lYear == rYear) {
						result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
					} else {
						result = lYear < rYear;
					}
				} else {
					result = lYear > rYear;
				}
			} else {
				result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		} else {
			result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	case Miam::IT_Disc:
		if (rType == Miam::IT_Disc) {
			int dLeft = left->data(Miam::DF_DiscNumber).toInt();
			int dRight = right->data(Miam::DF_DiscNumber).toInt();
			result = (dLeft < dRight && sortOrder() == Qt::AscendingOrder) ||
					  (dRight < dLeft && sortOrder() == Qt::DescendingOrder);
		}
		break;

	case Miam::IT_Separator:
		// Separators have a different sorting order when Hierarchical Order starts with Years
		if (SettingsPrivate::instance()->insertPolicy() == SettingsPrivate::IP_Years) {
			if (sortOrder() == Qt::AscendingOrder) {
				result = left->data(Miam::DF_NormalizedString).toInt() <= right->data(Miam::DF_NormalizedString).toInt();
			} else {
				result = left->data(Miam::DF_NormalizedString).toInt() + 10 <= right->data(Miam::DF_NormalizedString).toInt();
			}
		} else {
			// Special case if an artist's name has only one character, be sure to put it after the separator
			// Example: M (or -M-, or Mathieu Chedid)
			if (QString::compare(left->data(Miam::DF_NormalizedString).toString(),
								 right->data(Miam::DF_NormalizedString).toString().left(1)) == 0) {
				result = (sortOrder() == Qt::AscendingOrder);
			} else if (left->data(Miam::DF_NormalizedString).toString() == "0" && sortOrder() == Qt::DescendingOrder) {
				// Again a very special case to keep the separator for "Various" on top of siblings
				result = "9" < right->data(Miam::DF_NormalizedString).toString().left(1);
			} else {
				result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		}
		break;

	// Sort tracks by their numbers
	case Miam::IT_Track: {
		int dLeft = left->data(Miam::DF_DiscNumber).toInt();
		int lTrackNumber = left->data(Miam::DF_TrackNumber).toInt();
		int dRight = right->data(Miam::DF_DiscNumber).toInt();
		if (rType == Miam::IT_Track) {
			int rTrackNumber = right->data(Miam::DF_TrackNumber).toInt();
			if (dLeft == dRight) {
				// If there are both remote and local tracks under the same album, display first tracks from hard disk
				// Otherwise tracks will be displayed like #1 - local, #1 - remote, #2 - local, #2 - remote, etc
				bool lIsRemote = left->data(Miam::DF_IsRemote).toBool();
				bool rIsRemote = right->data(Miam::DF_IsRemote).toBool();
				if ((lIsRemote && rIsRemote) || (!lIsRemote && !rIsRemote)) {
					result = (lTrackNumber < rTrackNumber && sortOrder() == Qt::AscendingOrder) ||
						(rTrackNumber < lTrackNumber && sortOrder() == Qt::DescendingOrder);
				} else {
					result = (rIsRemote && sortOrder() == Qt::AscendingOrder) ||
						(lIsRemote && sortOrder() == Qt::DescendingOrder);
				}
			} else {
				result = (dLeft < dRight && sortOrder() == Qt::AscendingOrder) ||
						  (dRight < dLeft && sortOrder() == Qt::DescendingOrder);
			}
		} else if (rType == Miam::IT_Disc) {
			result = (dLeft < dRight && sortOrder() == Qt::AscendingOrder) ||
					  (dRight < dLeft && sortOrder() == Qt::DescendingOrder);
		} else {
			result = MiamSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;
	}
	case Miam::IT_Year: {
		int lYear = left->data(Miam::DF_NormalizedString).toInt();
		int rYear = right->data(Miam::DF_NormalizedString).toInt();
		result = (lYear < rYear && sortOrder() == Qt::AscendingOrder) ||
				  (rYear > lYear && sortOrder() == Qt::DescendingOrder);
		break;
	}
	default:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}
	return result;
}

bool LibraryFilterProxyModel::filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const
{
	return MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
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
