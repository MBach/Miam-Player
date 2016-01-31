#include "uniquelibraryitemmodel.h"

#include <model/sqldatabase.h>
#include <albumitem.h>
#include <artistitem.h>
#include <trackitem.h>
#include "coveritem.h"

#include <QSqlQuery>
#include <QSqlRecord>

#include <QtDebug>

UniqueLibraryItemModel::UniqueLibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new UniqueLibraryFilterProxyModel(this))
{
	setColumnCount(2);
	_proxy->setSourceModel(this);
	this->load();
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

void UniqueLibraryItemModel::load()
{
	this->deleteCache();

	SqlDatabase db;
	db.open();

	QSqlQuery query(db);
	query.setForwardOnly(true);
	if (query.exec("SELECT id, name, normalizedName, icon host FROM artists")) {
		QList<ArtistDAO> artists;
		while (query.next()) {
			ArtistDAO artist;
			int i = -1;
			artist.setId(query.record().value(++i).toString());
			artist.setTitle(query.record().value(++i).toString());
			artist.setTitleNormalized(query.record().value(++i).toString());
			artist.setIcon(query.record().value(++i).toString());
			artist.setHost(query.record().value(++i).toString());
			artists.append(artist);
		}
		insertArtists(artists);
	}

	if (query.exec("select alb.id, a.normalizedName || '|' || alb.year  || '|' || alb.normalizedName as merged, "\
				   "alb.name, a.name, alb.year, alb.host, alb.icon, cover " \
				   "from artists a " \
				   "inner join albums alb on a.id = alb.artistId")) {
		QList<AlbumDAO> albums;
		while (query.next()) {
			AlbumDAO album;
			int i = -1;
			album.setId(query.record().value(++i).toString());
			album.setTitleNormalized(query.record().value(++i).toString());
			album.setTitle(query.record().value(++i).toString());
			album.setArtist(query.record().value(++i).toString());
			album.setYear(query.record().value(++i).toString());
			album.setHost(query.record().value(++i).toString());
			album.setIcon(query.record().value(++i).toString());
			album.setCover(query.record().value(++i).toString());
			albums.append(album);
		}
		this->insertAlbums(albums);
	}

	query.prepare("SELECT art.normalizedName || '|' || alb.year  || '|' || alb.normalizedName || '|' || t.trackNumber  || '|' || t.title as merged, " \
				  "t.uri, t.trackNumber, t.title, art.name, alb.name, t.length, t.rating, t.disc, t.host, t.icon " \
				  "FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
				  "INNER JOIN artists art ON t.artistId = art.id");
	if (query.exec()) {
		QList<TrackDAO> tracks;
		while (query.next()) {
			TrackDAO track;
			int i = -1;
			track.setTitleNormalized(query.record().value(++i).toString());
			track.setUri(query.record().value(++i).toString());
			track.setTrackNumber(query.record().value(++i).toString());
			track.setTitle(query.record().value(++i).toString());
			track.setArtist(query.record().value(++i).toString());
			track.setAlbum(query.record().value(++i).toString());
			track.setLength(query.record().value(++i).toString());
			track.setRating(query.record().value(++i).toInt());
			track.setDisc(query.record().value(++i).toString());
			track.setHost(query.record().value(++i).toString());
			tracks.append(track);
		}
		this->insertTracks(tracks);
	}

	db.close();
}
