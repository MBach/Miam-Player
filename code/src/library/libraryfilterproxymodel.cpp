#include "libraryfilterproxymodel.h"

#include "libraryitem.h"
#include "librarymodel.h"
#include "settings.h"

#include <QtDebug>

#include <QPainter>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	this->setSortCaseSensitivity(Qt::CaseInsensitive);
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
bool LibraryFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool result;
	LibraryModel *model = qobject_cast<LibraryModel *>(this->sourceModel());
	LibraryItem *libraryItemLeft = model->itemFromIndex(left);
	LibraryItem *libraryItemRight = NULL;

	LibraryItemTrack *leftTrack = NULL;
	LibraryItemTrack *rightTrack = NULL;

	switch (libraryItemLeft->type()) {

	case LibraryItem::Artist:
		libraryItemRight = model->itemFromIndex(right);
		result = QSortFilterProxyModel::lessThan(left, right);
		break;

	case LibraryItem::Album:
		libraryItemRight = model->itemFromIndex(right);
		if (libraryItemRight && libraryItemRight->type() == LibraryItem::Album) {
			if (libraryItemLeft->data(Qt::UserRole+5).toInt() > 0 && libraryItemRight->data(Qt::UserRole+5).toInt() > 0) {
				result = libraryItemLeft->data(Qt::UserRole+5).toInt() < libraryItemRight->data(Qt::UserRole+5).toInt();
			} else {
				result = QSortFilterProxyModel::lessThan(left, right);
			}
		} else {
			result = QSortFilterProxyModel::lessThan(left, right);
		}
		break;

	case LibraryItem::Letter:
		libraryItemRight = model->itemFromIndex(right);
		// Special case if an artist's name has only one character, be sure to put it after the separator
		// Example: M (or -M-, or Mathieu Chedid)
		if (libraryItemRight && libraryItemRight->type() == LibraryItem::Artist && libraryItemLeft->text().compare(libraryItemRight->text()) == 0) {
			result = true;
		} else {
			result = QSortFilterProxyModel::lessThan(left, right);
		}
		break;

	// Sort tracks by their numbers
	case LibraryItem::Track:
		leftTrack = static_cast<LibraryItemTrack *>(model->itemFromIndex(left));
		rightTrack = static_cast<LibraryItemTrack *>(model->itemFromIndex(right));
		if (leftTrack && rightTrack) {
			result = leftTrack->trackNumber() < rightTrack->trackNumber();
		} else {
			result = QSortFilterProxyModel::lessThan(left, right);
		}
		break;

	default:
		result = QSortFilterProxyModel::lessThan(left, right);
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

/** Load covers only when an item needs to be expanded. */
void LibraryFilterProxyModel::loadCovers(const QModelIndex &index)
{
	LibraryModel *model = qobject_cast<LibraryModel *>(this->sourceModel());
	LibraryItem *artist = model->itemFromIndex(mapToSource(index));
	if (artist && artist->type() == LibraryItem::Artist) {
		Settings *settings = Settings::getInstance();
		if (settings->withCovers()) {

			// Load covers in a buffer greater than the real displayed picture
			int bufferedCoverSize = settings->bufferedCoverSize();
			QSize size(bufferedCoverSize, bufferedCoverSize);
			QPixmap pixmap(size);
			QPainter painter(&pixmap);

			for (int i=0; i < this->rowCount(index); i++) {

				// Build the path to the cover
				QModelIndex album = index.child(i, 0);
				int indexToAbsPath = album.data(LibraryItem::IDX_TO_ABS_PATH).toInt();
				QString absolutePath = settings->musicLocations().at(indexToAbsPath).toString();
				QString relativePathToCover = album.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
				QString coverPath = absolutePath + "/" + relativePathToCover;

				QStandardItem *item = model->itemFromIndex(this->mapToSource(album));
				// If the cover is still on the disk
				if (item && !relativePathToCover.isEmpty() && QFile::exists(coverPath)) {
					QImage image(coverPath);
					painter.drawImage(QRect(0, 0, bufferedCoverSize, bufferedCoverSize), image);
					item->setIcon(QIcon(pixmap));
				}
			}
		}
	}
}
