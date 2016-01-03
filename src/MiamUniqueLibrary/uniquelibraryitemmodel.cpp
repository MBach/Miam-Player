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

void UniqueLibraryItemModel::insertTracks(const QList<TrackDAO> nodes)
{
	QList<QStandardItem*> items;
	items.reserve(nodes.size());
	for (TrackDAO track : nodes) {
		TrackItem *item = new TrackItem(&track);
		items.append(item);
	}
	this->invisibleRootItem()->appendRows(items);
	this->proxy()->sort(0);
	this->proxy()->setDynamicSortFilter(true);
}

void UniqueLibraryItemModel::insertAlbums(const QList<AlbumDAO> nodes)
{
	QList<QStandardItem*> items;
	items.reserve(nodes.size());
	for (AlbumDAO album : nodes) {
		AlbumItem *item = new AlbumItem(&album);
		items.append(item);
	}
	this->invisibleRootItem()->appendRows(items);
}

void UniqueLibraryItemModel::insertArtists(const QList<ArtistDAO> nodes)
{
	QList<QStandardItem*> items;
	items.reserve(nodes.size());
	for (ArtistDAO artist : nodes) {
		ArtistItem *item = new ArtistItem(&artist);
		items.append(item);
	}
	this->invisibleRootItem()->appendRows(items);
}
