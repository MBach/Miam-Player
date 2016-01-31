#include "libraryitemmodel.h"

#include <settingsprivate.h>
#include <model/albumdao.h>
#include <model/artistdao.h>
#include <model/sqldatabase.h>
#include <model/trackdao.h>
#include <model/yeardao.h>
#include "albumitem.h"
#include "artistitem.h"
#include "trackitem.h"
#include "yearitem.h"

#include <functional>

#include <QSqlQuery>
#include <QSqlRecord>

#include <QtDebug>

LibraryItemModel::LibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new LibraryFilterProxyModel(this))
{
	setColumnCount(1);
	_proxy->setSourceModel(this);
	_proxy->setTopLevelItems(this->topLevelItems());
}

/** Read all tracks entries in the database and send them to connected views. */
void LibraryItemModel::load()
{
	//if (sendResetSignal) {
	//	emit aboutToLoad();
	//}

	SqlDatabase db;
	db.open();

	// Lambda function to reduce duplicate code which is relevant only in this method
	auto loadTracks = [this] (QSqlQuery& qTracks, AlbumDAO *albumDAO, const QString &year) -> void {
		bool internalCover = false;
		while (qTracks.next()) {
			QSqlRecord r = qTracks.record();
			TrackDAO *trackDAO = new TrackDAO;
			QString uri = r.value(0).toString();
			trackDAO->setUri(uri);
			trackDAO->setTrackNumber(r.value(1).toString());
			trackDAO->setTitle(r.value(2).toString());
			trackDAO->setArtist(r.value(3).toString());
			trackDAO->setAlbum(r.value(4).toString());
			trackDAO->setArtistAlbum(r.value(5).toString());
			trackDAO->setLength(r.value(6).toString());
			trackDAO->setRating(r.value(7).toInt());
			trackDAO->setDisc(r.value(8).toString());
			if (!internalCover && r.value(9).toBool()) {
				albumDAO->setCover(uri);
				internalCover = true;
			}
			trackDAO->setHost(r.value(10).toString());
			trackDAO->setIcon(r.value(11).toString());
			trackDAO->setParentNode(albumDAO);
			trackDAO->setYear(year);
			//emit nodeExtracted(trackDAO);
			this->insertNode(trackDAO);
		}
		if (internalCover) {
			// Cover path is now pointing to the first track of this album, because it need to be extracted at runtime
			//emit aboutToUpdateNode(albumDAO);
			this->updateNode(albumDAO);
		}
	};

	auto s = SettingsPrivate::instance();
	switch (s->insertPolicy()) {
	case SettingsPrivate::IP_Artists: {
		QStringList filters;
		if (s->isLibraryFilteredByArticles() && !s->libraryFilteredByArticles().isEmpty()) {
			filters = s->libraryFilteredByArticles();
		}

		// Level 1: Artists
		QSqlQuery qArtists("SELECT id, name, normalizedName FROM artists", db);
		qArtists.setForwardOnly(true);
		if (qArtists.exec()) {
			while (qArtists.next()) {
				QSqlRecord record = qArtists.record();
				ArtistDAO *artistDAO = new ArtistDAO;
				uint artistId = record.value(0).toUInt();
				QString artist = record.value(1).toString();
				artistDAO->setId(record.value(0).toString());
				artistDAO->setTitle(artist);
				if (filters.isEmpty()) {
					artistDAO->setTitleNormalized(record.value(2).toString());
				} else {
					for (QString filter : filters) {
						if (artist.startsWith(filter + " ", Qt::CaseInsensitive)) {
							artist = artist.mid(filter.length() + 1);
							artistDAO->setCustomData(artist + ", " + filter);
							break;
						}
					}
					artistDAO->setTitleNormalized(db.normalizeField(artist));
				}
				//emit nodeExtracted(artistDAO);
				this->insertNode(artistDAO);

				// Level 2: Albums
				QSqlQuery qAlbums(db);
				qAlbums.setForwardOnly(true);
				qAlbums.prepare("SELECT name, normalizedName, year, cover, host, icon, id FROM albums WHERE artistId = ?");
				qAlbums.addBindValue(artistId);
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO *albumDAO = new AlbumDAO;
						QString album = r.value(0).toString();
						albumDAO->setTitle(album);
						albumDAO->setTitleNormalized(r.value(1).toString());
						QString year = r.value(2).toString();
						albumDAO->setYear(year);
						albumDAO->setCover(r.value(3).toString());
						albumDAO->setHost(r.value(4).toString());
						albumDAO->setIcon(r.value(5).toString());
						uint albumId = r.value(6).toUInt();
						albumDAO->setParentNode(artistDAO);
						albumDAO->setArtist(artistDAO->title());
						albumDAO->setId(QString::number(albumId));
						//emit nodeExtracted(albumDAO);
						this->insertNode(albumDAO);

						//_cache.insert(albumId, albumDAO);

						// Level 3: Tracks
						QSqlQuery qTracks(db);
						qTracks.setForwardOnly(true);
						qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
										"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
										"INNER JOIN artists art ON t.artistId = art.id " \
										"WHERE t.artistId = ? AND t.albumId = ?");
						qTracks.addBindValue(artistId);
						qTracks.addBindValue(albumId);
						if (qTracks.exec()) {
							loadTracks(qTracks, albumDAO, year);
						}
					}
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_Albums: {
		// Level 1: Albums
		QSqlQuery qAlbums("SELECT name, normalizedName, year, cover, host, icon, id FROM albums", db);
		qAlbums.setForwardOnly(true);
		if (qAlbums.exec()) {
			while (qAlbums.next()) {
				QSqlRecord r = qAlbums.record();
				AlbumDAO *albumDAO = new AlbumDAO;
				QString album = r.value(0).toString();
				albumDAO->setTitle(album);
				albumDAO->setTitleNormalized(r.value(1).toString());
				QString year = r.value(2).toString();
				albumDAO->setYear(year);
				albumDAO->setCover(r.value(3).toString());
				albumDAO->setHost(r.value(4).toString());
				albumDAO->setIcon(r.value(5).toString());
				uint albumId = r.value(6).toUInt();
				//emit nodeExtracted(albumDAO);
				this->insertNode(albumDAO);

				// Level 2: Tracks
				QSqlQuery qTracks(db);
				qTracks.setForwardOnly(true);
				qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
								"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
								"INNER JOIN artists art ON t.artistId = art.id " \
								"WHERE t.albumId = ?");
				qTracks.addBindValue(albumId);
				if (qTracks.exec()) {
					loadTracks(qTracks, albumDAO, year);
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_ArtistsAlbums: {
		// Level 1: Artist - Album
		QSqlQuery qAlbums("SELECT art.name || ' – ' || alb.name, art.normalizedName || alb.normalizedName, alb.year, alb.cover, alb.host, alb.icon, alb.id " \
						  "FROM albums alb " \
						  "INNER JOIN artists art ON alb.artistId = art.id", db);
		qAlbums.setForwardOnly(true);
		if (qAlbums.exec()) {
			while (qAlbums.next()) {
				QSqlRecord r = qAlbums.record();
				AlbumDAO *albumDAO = new AlbumDAO;
				QString album = r.value(0).toString();
				albumDAO->setTitle(album);
				albumDAO->setTitleNormalized(r.value(1).toString());
				QString year = r.value(2).toString();
				albumDAO->setYear(year);
				albumDAO->setCover(r.value(3).toString());
				albumDAO->setHost(r.value(4).toString());
				albumDAO->setIcon(r.value(5).toString());
				uint albumId = r.value(6).toUInt();
				//emit nodeExtracted(albumDAO);
				this->insertNode(albumDAO);

				// Level 2: Tracks
				QSqlQuery qTracks(db);
				qTracks.setForwardOnly(true);
				qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
								"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
								"INNER JOIN artists art ON t.artistId = art.id " \
								"WHERE t.albumId = ?");
				qTracks.addBindValue(albumId);
				if (qTracks.exec()) {
					loadTracks(qTracks, albumDAO, year);
				}
			}
		}
		break;
	}
	case SettingsPrivate::IP_Years: {
		// Level 1: Years
		QSqlQuery qYears("SELECT DISTINCT year FROM albums ORDER BY year", db);
		qYears.setForwardOnly(true);
		if (qYears.exec()) {
			while (qYears.next()) {
				QSqlRecord r = qYears.record();
				YearDAO *yearDAO = new YearDAO;
				QVariant vYear = r.value(0);
				yearDAO->setTitle(vYear.toString());
				yearDAO->setTitleNormalized(vYear.toString());
				//emit nodeExtracted(yearDAO);
				this->insertNode(yearDAO);

				// Level 2: Artist - Album
				QSqlQuery qAlbums(db);
				qAlbums.setForwardOnly(true);
				qAlbums.prepare("SELECT art.name || ' – ' || alb.name, art.normalizedName || alb.normalizedName, alb.year, alb.cover, alb.host, alb.icon, art.id, alb.id " \
								"FROM albums alb INNER JOIN artists art ON alb.artistId = art.id " \
								"WHERE alb.year = ?");
				qAlbums.addBindValue(vYear.toInt());
				if (qAlbums.exec()) {
					while (qAlbums.next()) {
						QSqlRecord r = qAlbums.record();
						AlbumDAO *albumDAO = new AlbumDAO;
						QString album = r.value(0).toString();
						albumDAO->setTitle(album);
						albumDAO->setTitleNormalized(r.value(1).toString());
						QString year = r.value(2).toString();
						albumDAO->setYear(year);
						albumDAO->setCover(r.value(3).toString());
						albumDAO->setHost(r.value(4).toString());
						albumDAO->setIcon(r.value(5).toString());
						uint artistId = r.value(6).toUInt();
						uint albumId = r.value(7).toUInt();
						albumDAO->setParentNode(yearDAO);
						//emit nodeExtracted(albumDAO);
						this->insertNode(albumDAO);

						// Level 3: Tracks
						QSqlQuery qTracks(db);
						qTracks.setForwardOnly(true);
						qTracks.prepare("SELECT uri, trackNumber, title, art.name AS artist, alb.name AS album, artistAlbum, length, rating, disc, internalCover, " \
										"t.host, t.icon FROM tracks t INNER JOIN albums alb ON t.albumId = alb.id " \
										"INNER JOIN artists art ON t.artistId = art.id " \
										"WHERE t.artistId = ? AND t.albumId = ?");
						qTracks.addBindValue(artistId);
						qTracks.addBindValue(albumId);
						if (qTracks.exec()) {
							loadTracks(qTracks, albumDAO, year);
						}
					}
				}
			}
		}
		break;
	}
	}

	db.close();
	this->sort(0);
	//emit loaded();
}

/** For every item in the library, gets the top level letter attached to it. */
QChar LibraryItemModel::currentLetter(const QModelIndex &iTop) const
{
	QStandardItem *item = itemFromIndex(_proxy->mapToSource(iTop));

	// Special item "Various" (on top) has no Normalized String
	if (item && item->type() == Miam::IT_Separator && iTop.data(Miam::DF_NormalizedString).toString() == "0") {
		return QChar();
	} else if (!iTop.isValid()) {
		return QChar();
	} else {
		QModelIndex m = iTop;
		// An item without a valid parent is a top level item, therefore we can extract the letter.
		while (m.parent().isValid()) {
			m = m.parent();
		}
		if (m.isValid() && !m.data(Miam::DF_NormalizedString).toString().isEmpty()) {
			return m.data(Miam::DF_NormalizedString).toString().toUpper().at(0);
		} else {
			return QChar();
		}
	}
}

LibraryFilterProxyModel* LibraryItemModel::proxy() const
{
	return _proxy;
}

/** Rebuild the list of separators when one has changed grammatical articles in options. */
void LibraryItemModel::rebuildSeparators()
{
	SqlDatabase db;
	auto s = SettingsPrivate::instance();
	QStringList filters;
	if (s->isLibraryFilteredByArticles() && !s->libraryFilteredByArticles().isEmpty()) {
		filters = s->libraryFilteredByArticles();
	}

	// Reset custom displayed text, like "Artist, the"
	QHashIterator<SeparatorItem*, QModelIndex> i(_topLevelItems);
	while (i.hasNext()) {
		i.next();
		if (auto item = itemFromIndex(i.value())) {
			if (!i.value().data(Miam::DF_CustomDisplayText).toString().isEmpty()) {
				item->setData(QString(), Miam::DF_CustomDisplayText);
				// Recompute standard normalized name: "The Artist" -> "theartist"
				item->setData(db.normalizeField(item->text()), Miam::DF_NormalizedString);
			} else if (!filters.isEmpty()) {
				for (QString filter : filters) {
					QString text = item->text();
					if (text.startsWith(filter + " ", Qt::CaseInsensitive)) {
						text = text.mid(filter.length() + 1);
						item->setData(text + ", " + filter, Miam::DF_CustomDisplayText);
						item->setData(db.normalizeField(text), Miam::DF_NormalizedString);
						break;
					}
				}
			}
		}
	}

	// Delete separators first
	QSet<int> setRows;
	QHashIterator<QString, SeparatorItem*> it(_letters);
	while (it.hasNext()) {
		it.next();
		setRows << it.value()->index().row();
	}

	// Always remove items (rows) in reverse order
	QList<int> rows = setRows.toList();
	std::sort(rows.begin(), rows.end(), std::greater<int>());
	for (int row : rows) {
		auto item = takeItem(row);
		removeRow(row);
		delete item;
	}
	_letters.clear();
	_topLevelItems.clear();

	// Insert once again new separators
	for (int row = 0; row < rowCount(); row++) {
		auto item = this->item(row);
		if (item->type() != Miam::IT_Separator) {
			if (auto separator = this->insertSeparator(item)) {
				_topLevelItems.insert(separator, item->index());
			}
		}
	}
}

void LibraryItemModel::reset()
{
	_letters.clear();
	_topLevelItems.clear();
	_hash.clear();
	_tracks.clear();

	removeRows(0, rowCount());
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Artists:
		horizontalHeaderItem(0)->setText(tr("  Artists \\ Albums"));
		break;
	case SettingsPrivate::IP_Albums:
		horizontalHeaderItem(0)->setText(tr("  Albums"));
		break;
	case SettingsPrivate::IP_ArtistsAlbums:
		horizontalHeaderItem(0)->setText(tr("  Artists – Albums"));
		break;
	case SettingsPrivate::IP_Years:
		horizontalHeaderItem(0)->setText(tr("  Years"));
		break;
	}
}

void LibraryItemModel::cleanDanglingNodes()
{
	/// XXX: there's an empty row sometimes caused by extra SeparatorItem
	this->rebuildSeparators();
}

/** Find and insert a node in the hierarchy of items. */
void LibraryItemModel::insertNode(GenericDAO *node)
{
	if (!node) {
		return;
	}
	if (node && _hash.contains(node->hash())) {
		node->deleteLater();
		return;
	}

	QStandardItem *nodeItem = nullptr;
	if (TrackDAO *dao = qobject_cast<TrackDAO*>(node)) {
		nodeItem = new TrackItem(dao);
		if (_tracks.contains(dao->uri())) {
			QStandardItem *rowToDelete = _tracks.value(dao->uri());
			// Clean unused nodes
			this->removeNode(rowToDelete->index());
		}
		_tracks.insert(dao->uri(), nodeItem);
	} else if (AlbumDAO *dao = qobject_cast<AlbumDAO*>(node)) {
		AlbumItem *album = static_cast<AlbumItem*>(_hash.value(dao->hash()));
		if (album) {
			nodeItem = album;
		} else {
			nodeItem = new AlbumItem(dao);
		}
	} else if (ArtistDAO *dao = qobject_cast<ArtistDAO*>(node)) {
		ArtistItem *artist = static_cast<ArtistItem*>(_hash.value(dao->hash()));
		if (artist) {
			nodeItem = artist;
		} else {
			nodeItem = new ArtistItem(dao);
		}
	} else if (YearDAO *dao = qobject_cast<YearDAO*>(node)) {
		nodeItem = new YearItem(dao);
	}

	if (node->parentNode()) {
		if (QStandardItem *parentItem = _hash.value(node->parentNode()->hash())) {
			parentItem->appendRow(nodeItem);
		}
	} else if (nodeItem){
		invisibleRootItem()->appendRow(nodeItem);
		if (nodeItem->type() != Miam::IT_Separator) {
			if ( SeparatorItem *separator = this->insertSeparator(nodeItem)) {
				_topLevelItems.insert(separator, nodeItem->index());
			}
		}
	}
	if (nodeItem) {
		_hash.insert(node->hash(), nodeItem);
	} else {
		delete node;
	}
}
