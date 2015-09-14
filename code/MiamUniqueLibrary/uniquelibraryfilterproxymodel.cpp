#include "uniquelibraryfilterproxymodel.h"

#include <miamcore_global.h>
#include <settingsprivate.h>
#include <QStandardItem>

#include <albumitem.h>

UniqueLibraryFilterProxyModel::UniqueLibraryFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->setSortRole(Miam::DF_NormalizedString);
	this->setDynamicSortFilter(false);
	this->sort(0, Qt::AscendingOrder);}

bool UniqueLibraryFilterProxyModel::lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const
{
	bool result = false;
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->sourceModel());
	QStandardItem *left = model->itemFromIndex(idxLeft);
	QStandardItem *right = model->itemFromIndex(idxRight);

	int lType = left->type();
	int rType = right->type();
	switch (lType) {
	case Miam::IT_Artist:
		if (rType == Miam::IT_Artist) {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		} else if (rType == Miam::IT_Album) {
			// Same artist
			QString leftArtist = left->data(Miam::DF_NormalizedString).toString();
			QString rightArtist = right->data(Miam::DF_NormArtist).toString();
			if (leftArtist == rightArtist) {
				result = true;
			} else {
				if (sortOrder() == Qt::AscendingOrder) {
					result = leftArtist < rightArtist;
				} else {
					result = leftArtist > rightArtist;
				}
			}
		} else if (rType == Miam::IT_Track) {
			QString leftArtist = left->data(Miam::DF_NormalizedString).toString();
			QString rightArtist = right->data(Miam::DF_NormArtist).toString();
			if (leftArtist == rightArtist) {
				result = true;
			} else {
				if (sortOrder() == Qt::AscendingOrder) {
					result = leftArtist < rightArtist;
				} else {
					result = leftArtist > rightArtist;
				}
			}
		}// else {
		//	result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		//}
		break;

	case Miam::IT_Album:
		if (rType == Miam::IT_Album) {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		} else if (rType == Miam::IT_Track) {
			QString leftAlbum = left->data(Miam::DF_NormalizedString).toString();
			QString rightAlbum = right->data(Miam::DF_NormAlbum).toString();
			if (leftAlbum == rightAlbum) {
				result = true;
			} else {
				if (sortOrder() == Qt::AscendingOrder) {
					result = leftAlbum < rightAlbum;
				} else {
					result = leftAlbum > rightAlbum;
				}
			}
		}
		break;

	/*case Miam::IT_Disc:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;*/

	case Miam::IT_Separator:
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)
		if (QString::compare(left->data(Miam::DF_NormalizedString).toString(),
							 right->data(Miam::DF_NormalizedString).toString().left(1)) == 0) {
			result = (sortOrder() == Qt::AscendingOrder);
		} else if (left->data(Miam::DF_NormalizedString).toString() == "0" && sortOrder() == Qt::DescendingOrder) {
			// Again a very special case to keep the separator for "Various" on top of siblings
			result = "9" < right->data(Miam::DF_NormalizedString).toString().left(1);
		} else {
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;

	// Sort tracks by their numbers
	/*case Miam::IT_Track: {
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
			result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		}
		break;
	}*/
	default:
		result = QSortFilterProxyModel::lessThan(idxLeft, idxRight);
		break;
	}
	return result;
}
