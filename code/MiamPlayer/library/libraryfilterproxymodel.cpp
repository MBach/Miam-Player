#include "libraryfilterproxymodel.h"

#include "settingsprivate.h"
#include "../model/sqldatabase.h"

#include <QtDebug>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent), _topLevelItems(NULL)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(Miam::DF_NormalizedString);
	this->setDynamicSortFilter(false);
	this->sort(0, Qt::AscendingOrder);
}

/** Redefined to override Qt::FontRole. */
QVariant LibraryFilterProxyModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::FontRole) {
		return SettingsPrivate::instance()->font(SettingsPrivate::FF_Library);
	} else {
		return QSortFilterProxyModel::data(index, role);
	}
}

QStandardItem* LibraryFilterProxyModel::find(int level, const QString &nodeText) const
{
	for (int i = 0; i < rowCount(); i++) {
		QModelIndex ind = index(i, 0);
		if (ind.data().toString() == nodeText) {
			QModelIndex ind2 = mapToSource(ind);
			const QStandardItemModel *m = static_cast<const QStandardItemModel*>(ind2.model());
			return m->itemFromIndex(ind2);
		}
		for (int j = 0; j < rowCount(ind); j++) {
			if (level > 0)
				return this->find(level - 1, nodeText);
		}
	}
	return NULL;
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
			emit aboutToHighlight(sourceParent, true);
			//qDebug() << "accepting 1" << parent.data().toString();
			return true;
		}
		parent = parent.parent();
	}

	// Accept if any of the children is accepted on it's own merits
	if (hasAcceptedChildren(sourceRow, sourceParent)) {
		//qDebug() << "accepting 2" << sourceParent.data().toString();
		emit aboutToHighlight(sourceParent, true);
		return true;
	} else {
		//qDebug() << "refusing 1" << sourceParent.data().toString();
		emit aboutToHighlight(sourceParent, false);
	}

	// Accept separators if any top level items and its children are accepted
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(sourceModel());
	QStandardItem *item = model->itemFromIndex(model->index(sourceRow, 0, sourceParent));
	if (item && item->type() == Miam::IT_Separator) {
		foreach (QModelIndex index, _topLevelItems->values(item->index())) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				//qDebug() << "accepting Letter" << index.data().toString();
				emit aboutToHighlight(index, true);
				return true;
			}
		}
	}
	if (SettingsPrivate::instance()->isSearchAndExcludeLibrary()) {
		return false;
	} else {
		//qDebug() << "refusing 2" << sourceParent.data().toString();
		emit aboutToHighlight(sourceParent, false);
		return true;
	}
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
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;

	case Miam::IT_Album:
		if (rType == Miam::IT_Album) {
			int lYear = left->data(Miam::DF_Year).toInt();
			int rYear = right->data(Miam::DF_Year).toInt();
			if (SettingsPrivate::instance()->value("insertPolicy").toInt() == SqlDatabase::IP_Artists && lYear >= 0 && rYear >= 0) {
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

	case Miam::IT_Disc:
		if (rType == Miam::IT_Disc) {
			int dLeft = left->data(Miam::DF_DiscNumber).toInt();
			int dRight = right->data(Miam::DF_DiscNumber).toInt();
			result = (dLeft < dRight && sortOrder() == Qt::AscendingOrder) ||
					  (dRight < dLeft && sortOrder() == Qt::DescendingOrder);
		}
		break;

	case Miam::IT_Separator:
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)
		if (rType == Miam::IT_Artist || rType == Miam::IT_Album) {
			if (QString::compare(left->text().left(1), right->data(Miam::DF_NormalizedString).toString().left(1)) == 0) {
				result = (sortOrder() == Qt::AscendingOrder);
			} else {
				result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
			}
		} else { // IT_Year
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
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
				if (lIsRemote && rIsRemote || !lIsRemote && !rIsRemote) {
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
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
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
	/*if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
		emit aboutToHighlight(sourceParent, true);
		return true;
	} else {
		return false;
	}*/
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool LibraryFilterProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);
	if (!item.isValid()) {
		//qDebug() << "refusing A" << item.data().toString();
		emit aboutToHighlight(item, false);
		return false;
	}

	// Check if there are children
	int childCount = item.model()->rowCount(item);
	if (childCount == 0) {
		//qDebug() << "refusing B" << item.data().toString();
		emit aboutToHighlight(item, false);
		return false;
	}

	for (int i = 0; i < childCount; ++i) {
		if (filterAcceptsRowItself(i, item)) {
			//qDebug() << "accepting A" << item.child(i, 0).data().toString();
			emit aboutToHighlight(item.child(i, 0), true);
			return true;
		}
		// Recursive call
		if (hasAcceptedChildren(i, item)) {
			//qDebug() << "accepting B" << item.data().toString();
			emit aboutToHighlight(item, true);
			return true;
		}
	}
	//qDebug() << "refusing C" << item.data().toString();
	emit aboutToHighlight(item, false);
	return false;
}
