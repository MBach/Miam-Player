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

#include <QtDebug>

LibraryItemModel::LibraryItemModel(QObject *parent)
	: MiamItemModel(parent)
	, _proxy(new LibraryFilterProxyModel(this))
{
	setColumnCount(1);
	_proxy->setSourceModel(this);
	_proxy->setTopLevelItems(this->topLevelItems());
}

void LibraryItemModel::clearCache()
{
	_hash.clear();
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
	auto db = SqlDatabase::instance();
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
				item->setData(db->normalizeField(item->text()), Miam::DF_NormalizedString);
			} else if (!filters.isEmpty()) {
				for (QString filter : filters) {
					QString text = item->text();
					if (text.startsWith(filter + " ", Qt::CaseInsensitive)) {
						text = text.mid(filter.length() + 1);
						item->setData(text + ", " + filter, Miam::DF_CustomDisplayText);
						item->setData(db->normalizeField(text), Miam::DF_NormalizedString);
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
	if (!node || (node && _hash.contains(node->hash()))) {
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
	}
}
