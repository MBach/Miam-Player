#include "uniquelibraryitemmodel.h"

#include <albumitem.h>
#include <artistitem.h>
#include <trackitem.h>
#include "coveritem.h"

#include <QtDebug>

UniqueLibraryItemModel::UniqueLibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new UniqueLibraryFilterProxyModel(this))
{
	setColumnCount(2);
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

UniqueLibraryFilterProxyModel *UniqueLibraryItemModel::proxy() const
{
	return _proxy;
}

void UniqueLibraryItemModel::insertTracks(const QList<TrackDAO> nodes)
{
	for (TrackDAO track : nodes) {
		TrackItem *item = new TrackItem(&track);
		appendRow({ nullptr, item });
	}
	this->proxy()->sort(this->proxy()->defaultSortColumn());
	this->proxy()->setDynamicSortFilter(true);
}

void UniqueLibraryItemModel::insertAlbums(const QList<AlbumDAO> nodes)
{
	for (AlbumDAO album : nodes) {
		if (album.cover().isEmpty()) {
			appendRow({ nullptr, new AlbumItem(&album) });
		} else {
			appendRow({ new CoverItem(album.cover()), new AlbumItem(&album) });
		}
	}
}

void UniqueLibraryItemModel::insertArtists(const QList<ArtistDAO> nodes)
{
	for (ArtistDAO artist : nodes) {
		appendRow({ nullptr, new ArtistItem(&artist) });
	}
}
