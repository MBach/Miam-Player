#include "libraryitemmodel.h"

#include <settingsprivate.h>
#include <model/sqldatabase.h>
#include "albumitem.h"
#include "artistitem.h"
#include "trackitem.h"
#include "yearitem.h"

#include <functional>

#include <QRegularExpression>
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

LibraryItemModel::~LibraryItemModel()
{}

/** Read all tracks entries in the database and send them to connected views. */
void LibraryItemModel::load()
{
	qDebug() << Q_FUNC_INFO;
	this->reset();

	SqlDatabase db;
	db.init();

	QSqlQuery q("SELECT uri, trackNumber, trackTitle, artist, artistNormalized, album, albumNormalized, artistAlbum, " \
				"albumYear, trackLength, rating, disc, internalCover, cover, host, icon FROM cache", db);
	q.setForwardOnly(true);
	if (!q.exec()) {
		return;
	}

	// Lambda function to reduce duplicate code which is relevant in this method only
	auto loadTrack = [this] (QSqlRecord& r) -> TrackItem* {
		TrackItem *trackItem = new TrackItem;
		trackItem->setText(r.value(2).toString());
		trackItem->setData(r.value(0).toString(), Miam::DF_URI);
		trackItem->setData(r.value(1).toString(), Miam::DF_TrackNumber);
		trackItem->setData(r.value(11).toString(), Miam::DF_DiscNumber);
		trackItem->setData(r.value(9).toUInt(), Miam::DF_TrackLength);
		if (r.value(10).toInt() != -1) {
			trackItem->setData(r.value(10).toInt(), Miam::DF_Rating);
		}
		trackItem->setData(r.value(3).toString(), Miam::DF_Artist);
		trackItem->setData(r.value(5).toString(), Miam::DF_Album);
		trackItem->setData(!r.value(14).toString().isEmpty(), Miam::DF_IsRemote);
		return trackItem;
	};

	auto s = SettingsPrivate::instance();
	switch (s->insertPolicy()) {
	case SettingsPrivate::IP_Artists: {
		QStringList filters;
		if (s->isLibraryFilteredByArticles() && !s->libraryFilteredByArticles().isEmpty()) {
			filters = s->libraryFilteredByArticles();
		}

		QHash<uint, ArtistItem*> _artists;
		QHash<uint, AlbumItem*> _albums;

		while (q.next()) {
			QSqlRecord r = q.record();

			ArtistItem *artistItem = new ArtistItem;
			QString artistNormalized = r.value(4).toString();
			QString artist = r.value(3).toString();
			QString artistAlbum = r.value(7).toString();
			artistItem->setText(artistAlbum);
			for (QString filter : filters) {
				if (artist.startsWith(filter + " ", Qt::CaseInsensitive)) {
					artist = artist.mid(filter.length() + 1);
					artistItem->setData(artist + ", " + filter, Miam::DF_CustomDisplayText);
					break;
				}
			}

			if (artistNormalized.isEmpty() || !artistNormalized.contains(QRegularExpression("[\\w]"))) {
				artistItem->setData("0", Miam::DF_NormalizedString);
			} else {
				artistItem->setData(artistNormalized, Miam::DF_NormalizedString);
			}

			// Add artist
			if (_artists.contains(artistItem->hash())) {
				auto it = _artists.find(artistItem->hash());
				delete artistItem;
				artistItem = (*it);
			} else {
				_artists.insert(artistItem->hash(), artistItem);
				invisibleRootItem()->appendRow(artistItem);

				// Also check if newly inserted artist needs to insert a separator
				if (SeparatorItem *separator = this->insertSeparator(artistItem)) {
					_topLevelItems.insert(separator, artistItem->index());
				}
			}

			AlbumItem *albumItem = new AlbumItem;
			albumItem->setText(r.value(5).toString());
			if (r.value(6).toString().isEmpty() || !r.value(6).toString().contains(QRegularExpression("[\\w]"))) {
				albumItem->setData("0", Miam::DF_NormalizedString);
			} else {
				albumItem->setData(r.value(6).toString(), Miam::DF_NormalizedString);
			}
			albumItem->setData(artistNormalized, Miam::DF_NormArtist);
			albumItem->setData(r.value(8).toString(), Miam::DF_Year);
			albumItem->setData(r.value(13).toString(), Miam::DF_CoverPath);
			albumItem->setData(r.value(14).toString(), Miam::DF_IconPath);
			albumItem->setData(!r.value(12).toString().isEmpty(), Miam::DF_IsRemote);

			// Add album
			if (_albums.contains(albumItem->hash())) {
				auto it = _albums.find(albumItem->hash());
				delete albumItem;
				albumItem = *it;
			} else {
				_albums.insert(albumItem->hash(), albumItem);
				artistItem->appendRow(albumItem);
			}

			// Add tracks
			albumItem->appendRow(loadTrack(r));
		}
		break;
	}
	case SettingsPrivate::IP_Albums: {

		QHash<uint, AlbumItem*> _albums;
		while (q.next()) {
			QSqlRecord r = q.record();
			QString artistNormalized = r.value(4).toString();

			AlbumItem *albumItem = new AlbumItem;
			albumItem->setText(r.value(5).toString());
			if (r.value(6).toString().isEmpty() || !r.value(6).toString().contains(QRegularExpression("[\\w]"))) {
				albumItem->setData("0", Miam::DF_NormalizedString);
			} else {
				albumItem->setData(r.value(6).toString(), Miam::DF_NormalizedString);
			}
			albumItem->setData(artistNormalized, Miam::DF_NormArtist);
			albumItem->setData(r.value(8).toString(), Miam::DF_Year);
			albumItem->setData(r.value(13).toString(), Miam::DF_CoverPath);
			albumItem->setData(r.value(14).toString(), Miam::DF_IconPath);
			albumItem->setData(!r.value(12).toString().isEmpty(), Miam::DF_IsRemote);

			// Add album
			if (_albums.contains(albumItem->hash())) {
				auto it = _albums.find(albumItem->hash());
				delete albumItem;
				albumItem = (*it);
			} else {
				_albums.insert(albumItem->hash(), albumItem);
				invisibleRootItem()->appendRow(albumItem);
				// Also check if newly inserted artist needs to insert a separator
				if (SeparatorItem *separator = this->insertSeparator(albumItem)) {
					_topLevelItems.insert(separator, albumItem->index());
				}
			}

			// Add tracks
			albumItem->appendRow(loadTrack(r));
		}
		break;
	}
	case SettingsPrivate::IP_ArtistsAlbums: {
		QHash<uint, AlbumItem*> _albums;
		while (q.next()) {
			QSqlRecord r = q.record();
			QString artistNormalized = r.value(4).toString();
			QString albumNormalized = r.value(6).toString();

			AlbumItem *albumItem = new AlbumItem;
			albumItem->setText(r.value(3).toString() + " – " + r.value(5).toString());
			albumItem->setData(artistNormalized + "|" + albumNormalized, Miam::DF_NormalizedString);
			albumItem->setData(artistNormalized, Miam::DF_NormArtist);
			albumItem->setData(r.value(8).toString(), Miam::DF_Year);
			albumItem->setData(r.value(13).toString(), Miam::DF_CoverPath);
			albumItem->setData(r.value(14).toString(), Miam::DF_IconPath);
			albumItem->setData(!r.value(12).toString().isEmpty(), Miam::DF_IsRemote);

			// Add album
			if (_albums.contains(albumItem->hash())) {
				auto it = _albums.find(albumItem->hash());
				delete albumItem;
				albumItem = *it;
			} else {
				_albums.insert(albumItem->hash(), albumItem);
				invisibleRootItem()->appendRow(albumItem);
				// Also check if newly inserted artist needs to insert a separator
				if (SeparatorItem *separator = this->insertSeparator(albumItem)) {
					_topLevelItems.insert(separator, albumItem->index());
				}
			}

			// Add tracks
			albumItem->appendRow(loadTrack(r));
		}
		break;
	}
	case SettingsPrivate::IP_Years: {

		QHash<uint, YearItem*> _years;
		QHash<uint, AlbumItem*> _artistAlbums;

		while (q.next()) {
			QSqlRecord r = q.record();
			YearItem *yearItem = new YearItem(r.value(8).toString());

			// Add year
			if (_years.contains(yearItem->hash())) {
				auto it = _years.find(yearItem->hash());
				delete yearItem;
				yearItem = (*it);
			} else {
				_years.insert(yearItem->hash(), yearItem);
				invisibleRootItem()->appendRow(yearItem);

				// Also check if newly inserted artist needs to insert a separator
				if (SeparatorItem *separator = this->insertSeparator(yearItem)) {
					_topLevelItems.insert(separator, yearItem->index());
				}
			}

			// Add Artist - Album
			AlbumItem *artistAlbumItem = new AlbumItem;
			artistAlbumItem->setText(r.value(3).toString() + " – " + r.value(5).toString());
			artistAlbumItem->setData(r.value(4).toString() + "|" + r.value(6).toString(), Miam::DF_NormalizedString);
			artistAlbumItem->setData(r.value(4).toString(), Miam::DF_NormArtist);
			artistAlbumItem->setData(r.value(8).toString(), Miam::DF_Year);
			artistAlbumItem->setData(r.value(13).toString(), Miam::DF_CoverPath);
			artistAlbumItem->setData(r.value(14).toString(), Miam::DF_IconPath);
			artistAlbumItem->setData(!r.value(12).toString().isEmpty(), Miam::DF_IsRemote);

			if (_artistAlbums.contains(artistAlbumItem->hash())) {
				auto it = _artistAlbums.find(artistAlbumItem->hash());
				delete artistAlbumItem;
				artistAlbumItem = *it;
			} else {
				_artistAlbums.insert(artistAlbumItem->hash(), artistAlbumItem);
				yearItem->appendRow(artistAlbumItem);
			}

			// Add tracks
			artistAlbumItem->appendRow(loadTrack(r));
		}
		break;
	}
	}

	this->sort(0);
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
	this->deleteCache();
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
