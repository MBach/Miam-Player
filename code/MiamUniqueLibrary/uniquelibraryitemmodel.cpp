#include "uniquelibraryitemmodel.h"

#include <albumitem.h>
#include <artistitem.h>
#include <trackitem.h>

#include <QtDebug>

UniqueLibraryItemModel::UniqueLibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new UniqueLibraryFilterProxyModel(this))
{
	setColumnCount(1);
	_proxy->setSourceModel(this);
}

UniqueLibraryFilterProxyModel *UniqueLibraryItemModel::proxy() const
{
	return _proxy;
}

/** Find and insert a node in the hierarchy of items. */
void UniqueLibraryItemModel::insertNode(GenericDAO *node)
{
	if (!node || (node && _hash.contains(node->hash()))) {
		return;
	}

	QStandardItem *nodeItem = nullptr;
	if (TrackDAO *dao = qobject_cast<TrackDAO*>(node)) {
		nodeItem = new TrackItem(dao);
		QString normalized = dao->disc() + "|" + dao->trackNumber(true) + "|" + dao->title();
		AlbumDAO *album = static_cast<AlbumDAO*>(dao->parentNode());
		if (album) {
			normalized.prepend(album->year() + "|" + album->title() + "|");
			ArtistDAO *artist = static_cast<ArtistDAO*>(album->parentNode());
			if (artist) {
				normalized.prepend(artist->title() + "|");
			}
		}
		nodeItem->setData(normalized, Miam::DF_NormalizedString);
		//nodeItem->setData(normalized, Qt::DisplayRole);
		if (_tracks.contains(dao->uri())) {
			QStandardItem *rowToDelete = _tracks.value(dao->uri());
			// Clean unused nodes
			this->removeNode(rowToDelete->index());
		}
		_tracks.insert(dao->uri(), nodeItem);
	} else if (AlbumDAO *dao = qobject_cast<AlbumDAO*>(node)) {
		QString normalized = dao->year() + "|" + dao->title();
		ArtistDAO *artist = static_cast<ArtistDAO*>(dao->parentNode());
		if (artist) {
			normalized.prepend(artist->title() + "|");
		}
		AlbumItem *album = static_cast<AlbumItem*>(_hash.value(dao->hash()));
		if (album) {
			nodeItem = album;
		} else {
			nodeItem = new AlbumItem(dao);
		}
		nodeItem->setData(normalized, Miam::DF_NormalizedString);
		//nodeItem->setData(normalized, Qt::DisplayRole);

	} else if (ArtistDAO *dao = qobject_cast<ArtistDAO*>(node)) {
		ArtistItem *artist = static_cast<ArtistItem*>(_hash.value(dao->hash()));
		if (artist) {
			nodeItem = artist;
		} else {
			nodeItem = new ArtistItem(dao);
		}
		nodeItem->setData(dao->title() + "|", Miam::DF_NormalizedString);
		//nodeItem->setData(dao->title() + "|", Qt::DisplayRole);
	}

	if (nodeItem){
		invisibleRootItem()->appendRow(nodeItem);
		if (nodeItem->type() == Miam::IT_Artist) {
			if (SeparatorItem *separator = this->insertSeparator(nodeItem)) {
				_topLevelItems.insert(separator, nodeItem->index());
			}
		}
		_hash.insert(node->hash(), nodeItem);
	}
}
