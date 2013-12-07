#include "libraryfilterproxymodel.h"

#include <model/libraryitem.h>
#include "settings.h"

#include <QtDebug>

#include <QPainter>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(LibraryItem::NormalizedString);
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

/** Redefined for custom sorting. */
bool LibraryFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	/*LibraryModel *model = qobject_cast<LibraryModel *>(this->sourceModel());
	LibraryItem *left = model->itemFromIndex(idxLeft);
	LibraryItem *right = model->itemFromIndex(idxRight);

	LibraryItemAlbum *leftAlbum = NULL, *rightAlbum = NULL;
	LibraryItemTrack *leftTrack = NULL, *rightTrack = NULL;
	LibraryItemDiscNumber *leftDisc = NULL, *rightDisc = NULL;

	switch (left->type()) {

	case LibraryItem::Artist:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;

	case LibraryItem::Album:
		if (right && right->type() == LibraryItem::Album) {
			leftAlbum = static_cast<LibraryItemAlbum*>(model->itemFromIndex(idxLeft));
			rightAlbum = static_cast<LibraryItemAlbum*>(model->itemFromIndex(idxRight));
			if (Settings::getInstance()->insertPolicy() == Settings::Artist && leftAlbum->year() >= 0 && rightAlbum->year() >= 0) {
			//if (model->currentInsertPolicy() == Settings::Artist && leftAlbum->year() >= 0 && rightAlbum->year() >= 0) {
				if (sortOrder() == Qt::AscendingOrder) {
					if (leftAlbum->year() == rightAlbum->year()) {
						result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
					} else {
						result = leftAlbum->year() < rightAlbum->year();
					}
				} else {
					result = leftAlbum->year() > rightAlbum->year();
				}
			} else {
				result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	case LibraryItem::Disc:
		leftDisc = static_cast<LibraryItemDiscNumber *>(model->itemFromIndex(idxLeft));
		if (model->itemFromIndex(idxRight)->type() == LibraryItem::Disc) {
			rightDisc = static_cast<LibraryItemDiscNumber *>(model->itemFromIndex(idxRight));
			result = leftDisc->discNumber() < rightDisc->discNumber();
		} else if (model->itemFromIndex(idxRight)->type() == LibraryItem::Track) {
			rightTrack = static_cast<LibraryItemTrack *>(model->itemFromIndex(idxRight));
			result = leftDisc->discNumber() < rightTrack->discNumber();
		}
		break;

	case LibraryItem::Letter:
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)
		if (right && QString::compare(left->normalizedString().left(1), right->normalizedString().left(1)) == 0) {
			result = (sortOrder() == Qt::AscendingOrder);
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	// Sort tracks by their numbers
	case LibraryItem::Track:
		leftTrack = static_cast<LibraryItemTrack *>(model->itemFromIndex(idxLeft));
		rightTrack = static_cast<LibraryItemTrack *>(model->itemFromIndex(idxRight));
		if (rightTrack) {
			if (leftTrack->discNumber() == rightTrack->discNumber()) {
				result = (leftTrack->trackNumber() < rightTrack->trackNumber() && sortOrder() == Qt::AscendingOrder) ||
						(leftTrack->trackNumber() > rightTrack->trackNumber() && sortOrder() == Qt::DescendingOrder);
			} else {
				result = (leftTrack->discNumber() < rightTrack->discNumber() && sortOrder() == Qt::AscendingOrder) ||
						  (leftTrack->discNumber() > rightTrack->discNumber() && sortOrder() == Qt::DescendingOrder);
			}
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	default:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}*/
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
