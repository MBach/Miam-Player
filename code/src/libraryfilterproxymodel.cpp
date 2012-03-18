#include "libraryfilterproxymodel.h"

#include "libraryitem.h"
#include "librarymodel.h"
#include "settings.h"

#include <QtDebug>

#include <QPainter>

LibraryFilterProxyModel::LibraryFilterProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	//this->setHeaderData(0, Qt::Horizontal, QVariant("Artists"), Qt::DisplayRole);
}

bool LibraryFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (filterAcceptsRowItself(sourceRow, sourceParent)) {
		if (!filterRegExp().isEmpty()) {
			emit aboutToExpand(mapFromSource(sourceParent));
		}
		return true;
	}

	//accept if any of the parents is accepted on it's own merits
	QModelIndex parent = sourceParent;
	while (parent.isValid()) {
		if (filterAcceptsRowItself(parent.row(), parent.parent())) {
			return true;
		}
		parent = parent.parent();
	}

	//accept if any of the children is accepted on it's own merits
	if (hasAcceptedChildren(sourceRow, sourceParent)) {
		return true;
	}
	return false;
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

	//check if there are children
	int childCount = item.model()->rowCount(item);
	if (childCount == 0) {
		return false;
	}

	for (int i = 0; i < childCount; ++i) {
		if (filterAcceptsRowItself(i, item)) {
			return true;
		}
		//recursive call
		if (hasAcceptedChildren(i, item)) {
			return true;
		}
	}
	return false;
}

QVariant LibraryFilterProxyModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::FontRole) {
		return Settings::getInstance()->font(Settings::LIBRARY);
	} else {
		return QSortFilterProxyModel::data(index, role);
	}
}

/** Load covers only when an item need to be expanded. */
void LibraryFilterProxyModel::loadCovers(const QModelIndex &index)
{
	if (index.data(LibraryItem::MEDIA_TYPE) == LibraryModel::ARTIST) {

		Settings *settings = Settings::getInstance();
		LibraryModel *model = qobject_cast<LibraryModel *>(this->sourceModel());

		for (int i=0; i < this->rowCount(index); i++) {

			// Build the path to the cover
			QModelIndex album = index.child(i, 0);
			int indexToAbsPath = album.data(LibraryItem::IDX_TO_ABS_PATH).toInt();
			QString absolutePath = settings->musicLocations().at(indexToAbsPath).toString();
			QString relativePathToCover = album.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
			QString coverPath = absolutePath + "/" + relativePathToCover;

			// If the cover is still on the disk
			if (!relativePathToCover.isEmpty() && QFileInfo(coverPath).exists()) {

				QStandardItem *item = model->itemFromIndex(this->mapToSource(album));
				if (item && item->icon().isNull()) {
					QSize size(48, 48);	// change to value in settings
					QPixmap pixmap(size);
					QPainter painter(&pixmap);
					QImage image(coverPath);
					painter.drawImage(QRect(0, 0, size.width(), size.height()), image);
					item->setIcon(QIcon(pixmap));
				}
			}
		}
	}
}

/** Redefined for custom sorting. */
bool LibraryFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool result;
	LibraryModel *model = qobject_cast<LibraryModel *>(this->sourceModel());
	QStandardItem *qStandardItemLeft = model->itemFromIndex(left);

	LibraryItem *libraryItemLeft = dynamic_cast<LibraryItem *>(qStandardItemLeft);
	LibraryItem *libraryItemRight = NULL;

	switch (libraryItemLeft->mediaType()) {

	case LibraryModel::TRACK:
		libraryItemRight = dynamic_cast<LibraryItem *>(model->itemFromIndex(right));
		result = libraryItemLeft->trackNumber() < libraryItemRight->trackNumber();
		break;

	default:
		result = QSortFilterProxyModel::lessThan(left, right);
	}
	return result;
}
