#include "uniquelibraryitemmodel.h"

#include <model/sqldatabase.h>
#include <albumitem.h>
#include <artistitem.h>
#include <discitem.h>
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

void UniqueLibraryItemModel::load(const QString &filter)
{
	this->deleteCache();

	SqlDatabase db;

	QSqlQuery query(db);
	query.setForwardOnly(true);
	if (filter.isEmpty()) {
		query.prepare("SELECT DISTINCT artistAlbum, artistNormalized, icon, host FROM cache");
	} else {
		query.prepare("SELECT DISTINCT artistAlbum, artistNormalized, icon, host FROM cache " \
					  "WHERE trackTitle LIKE :t OR artist LIKE :ar OR album LIKE :al");
		query.bindValue(":t", "%" + filter + "%");
		query.bindValue(":ar", "%" + filter + "%");
		query.bindValue(":al", "%" + filter + "%");
	}
	if (query.exec()) {
		while (query.next()) {
			ArtistItem *artist = new ArtistItem;
			int i = -1;
			artist->setText(query.record().value(++i).toString());
			artist->setData(query.record().value(++i).toString(), Miam::DF_NormalizedString);
			artist->setData(query.record().value(++i).toString(), Miam::DF_IconPath);
			artist->setData(!query.record().value(++i).toString().isEmpty(), Miam::DF_IsRemote);
			appendRow({ nullptr, artist });
		}
	}

	if (filter.isEmpty()) {
		query.prepare("SELECT DISTINCT artistNormalized || '|' || albumYear  || '|' || albumNormalized, albumNormalized, album, artistAlbum, " \
					  "albumYear, icon, internalCover, cover FROM cache ORDER BY uri, internalCover");
	} else {
		query.prepare("SELECT DISTINCT artistNormalized || '|' || albumYear  || '|' || albumNormalized, albumNormalized, album, artistAlbum, " \
					  "albumYear, icon, internalCover, cover FROM cache WHERE trackTitle LIKE :t OR artist LIKE :ar OR album LIKE :al ORDER BY uri, internalCover");
		query.bindValue(":t", "%" + filter + "%");
		query.bindValue(":ar", "%" + filter + "%");
		query.bindValue(":al", "%" + filter + "%");
	}
	if (query.exec()) {
		QString normalizedStringPrevious;
		while (query.next()) {
			int i = -1;
			QString normalizedString = query.record().value(++i).toString();
			if (normalizedStringPrevious == normalizedString) {
				continue;
			} else {
				normalizedStringPrevious = normalizedString;
			}
			AlbumItem *album = new AlbumItem;
			album->setData(normalizedString, Miam::DF_NormalizedString);
			album->setData(query.record().value(++i).toString(), Miam::DF_NormAlbum);
			album->setText(query.record().value(++i).toString());
			album->setData(query.record().value(++i).toString(), Miam::DF_Artist);
			album->setData(query.record().value(++i).toString(), Miam::DF_Year);
			album->setData(query.record().value(++i).toString(), Miam::DF_IconPath);
			QString internalCover = query.record().value(++i).toString();
			QString coverPath = query.record().value(++i).toString();
			CoverItem *cover = nullptr;
			if (!internalCover.isEmpty() || !coverPath.isEmpty()) {
				cover = new CoverItem;
				if (internalCover.isEmpty()) {
					cover->setData(coverPath, Miam::DF_CoverPath);
				} else {
					cover->setData(internalCover, Miam::DF_InternalCover);
				}
			}
			appendRow({ cover, album });
		}
	}

	if (filter.isEmpty()) {
		query.prepare("SELECT DISTINCT artistNormalized || '|' || albumYear  || '|' || albumNormalized || '|' || substr('0' || disc, -1, 1)" \
					  ", artistAlbum, disc FROM cache WHERE disc > 0");
	} else {
		query.prepare("SELECT DISTINCT artistNormalized || '|' || albumYear  || '|' || albumNormalized || '|' || substr('0' || disc, -1, 1)" \
					  ", artistAlbum, disc FROM cache WHERE (disc > 0) AND (trackTitle LIKE :t OR artist LIKE :ar OR album LIKE :al)");
		query.bindValue(":t", "%" + filter + "%");
		query.bindValue(":ar", "%" + filter + "%");
		query.bindValue(":al", "%" + filter + "%");
	}
	if (query.exec()) {
		while (query.next()) {
			DiscItem *disc = new DiscItem;
			int i = -1;
			disc->setData(query.record().value(++i).toString(), Miam::DF_NormalizedString);
			disc->setData(query.record().value(++i).toString(), Miam::DF_Artist);
			disc->setText(query.record().value(++i).toString());
			appendRow({ nullptr, disc });
		}
	}

	if (filter.isEmpty()) {
		query.prepare("SELECT artistNormalized || '|' || albumYear  || '|' || albumNormalized || '|' || substr('0' || disc, -1, 1) || '|' || substr('00' || trackNumber, -2, 2)  || '|' || trackTitle, " \
					  "trackTitle, uri, trackNumber, artistAlbum, album, trackLength, rating, disc, host FROM cache");
	} else {
		query.prepare("SELECT artistNormalized || '|' || albumYear  || '|' || albumNormalized || '|' || substr('0' || disc, -1, 1) || '|' || substr('00' || trackNumber, -2, 2)  || '|' || trackTitle, " \
					  "trackTitle, uri, trackNumber, artistAlbum, album, trackLength, rating, disc, host FROM cache WHERE trackTitle LIKE :t OR artist LIKE :ar OR album LIKE :al");
		query.bindValue(":t", "%" + filter + "%");
		query.bindValue(":ar", "%" + filter + "%");
		query.bindValue(":al", "%" + filter + "%");
	}
	if (query.exec()) {
		while (query.next()) {
			TrackItem *track = new TrackItem;
			int i = -1;
			track->setData(query.record().value(++i).toString(), Miam::DF_NormalizedString);
			track->setText(query.record().value(++i).toString());
			track->setData(query.record().value(++i).toString(), Miam::DF_URI);
			track->setData(query.record().value(++i).toString(), Miam::DF_TrackNumber);
			track->setData(query.record().value(++i).toString(), Miam::DF_Artist);
			track->setData(query.record().value(++i).toString(), Miam::DF_Album);
			track->setData(query.record().value(++i).toUInt(), Miam::DF_TrackLength);
			track->setData(query.record().value(++i).toInt(), Miam::DF_Rating);
			track->setData(query.record().value(++i).toString(), Miam::DF_DiscNumber);
			track->setData(!query.record().value(++i).toString().isEmpty(), Miam::DF_IsRemote);
			appendRow({ nullptr, track });
		}

	}
	this->proxy()->sort(this->proxy()->defaultSortColumn());
	this->proxy()->setDynamicSortFilter(false);
}
