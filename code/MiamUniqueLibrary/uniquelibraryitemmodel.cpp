#include "uniquelibraryitemmodel.h"

#include <albumitem.h>
#include <artistitem.h>
#include <trackitem.h>

#include <QtDebug>

UniqueLibraryItemModel::UniqueLibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new MiamSortFilterProxyModel(this))
{
	setColumnCount(1);
	_proxy->setSourceModel(this);
}

QChar UniqueLibraryItemModel::currentLetter(const QModelIndex &index) const
{
	QStandardItem *item = itemFromIndex(_proxy->mapToSource(index));
	if (item && item->type() == Miam::IT_Separator && index.data(Miam::DF_NormalizedString).toString() == "0") {
		return QChar();
	} else if (!index.isValid()) {
		return QChar();
	} else {
		// An item without a valid parent is a top level item, therefore we can extract the letter.
		if (!index.data(Miam::DF_NormalizedString).toString().isEmpty()) {
			return index.data(Miam::DF_NormalizedString).toString().toUpper().at(0);
		} else {
			return QChar();
		}
	}
}

MiamSortFilterProxyModel *UniqueLibraryItemModel::proxy() const
{
	return _proxy;
}

/** Find and insert a node in the hierarchy of items. */
void UniqueLibraryItemModel::insertNode(GenericDAO *node)
{
	if (!node) {
		return;
	}

	QStandardItem *nodeItem = nullptr;
	if (TrackDAO *dao = qobject_cast<TrackDAO*>(node)) {
		nodeItem = new TrackItem(dao);
		QString normalized = dao->disc() + "|" + dao->trackNumber(true) + "|" + dao->title();
		QString customSort = dao->title();
		AlbumDAO *album = static_cast<AlbumDAO*>(dao->parentNode());
		if (album) {
			normalized.prepend(album->year() + "|" + album->title() + "|");
			customSort.prepend(album->title() + "|");
			ArtistDAO *artist = static_cast<ArtistDAO*>(album->parentNode());
			if (artist) {
				normalized.prepend(artist->titleNormalized() + "|");
				customSort.prepend(artist->title() + "|");
			}
		}
		nodeItem->setData(normalized, Miam::DF_NormalizedString);
		nodeItem->setData(customSort, Miam::DF_CustomSortRole);

		if (_tracks.contains(dao->uri())) {
			QStandardItem *rowToDelete = _tracks.value(dao->uri());
			// Clean unused nodes
			this->removeNode(rowToDelete->index());
		}
		_tracks.insert(dao->uri(), nodeItem);
	} else if (AlbumDAO *dao = qobject_cast<AlbumDAO*>(node)) {
		QString normalized = dao->year() + "|" + dao->title();
		QString customSort = dao->title();

		ArtistDAO *artist = static_cast<ArtistDAO*>(dao->parentNode());
		if (artist) {
			normalized.prepend(artist->titleNormalized() + "|");
			customSort.prepend(artist->title() + "|");
		}
		nodeItem = new AlbumItem(dao);
		nodeItem->setData(normalized, Miam::DF_NormalizedString);
		nodeItem->setData(customSort, Miam::DF_CustomSortRole);

	} else if (ArtistDAO *dao = qobject_cast<ArtistDAO*>(node)) {
		nodeItem = new ArtistItem(dao);
		nodeItem->setData(dao->titleNormalized() + "|", Miam::DF_NormalizedString);
		nodeItem->setData(dao->title(), Miam::DF_CustomSortRole);
	}

	if (nodeItem){
		invisibleRootItem()->appendRow(nodeItem);
		if (nodeItem->type() == Miam::IT_Artist) {
			if (SeparatorItem *separator = this->insertSeparator(nodeItem)) {
				_topLevelItems.insert(separator, nodeItem->index());
			}
		}
	}
}
