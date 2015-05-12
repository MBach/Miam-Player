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
	: QStandardItemModel(parent)
{
	setColumnCount(1);
}

void LibraryItemModel::clearCache()
{
	_hash.clear();
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

SeparatorItem *LibraryItemModel::insertSeparator(const QStandardItem *node)
{
	// Items are grouped every ten years in this particular case
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Years: {
		int year = node->text().toInt();
		if (year == 0) {
			return NULL;
		}
		QString yearStr = QString::number(year - year % 10);
		if (_letters.contains(yearStr)) {
			return _letters.value(yearStr);
		} else {
			SeparatorItem *separator = new SeparatorItem(yearStr);
			separator->setData(yearStr, Miam::DF_NormalizedString);
			invisibleRootItem()->appendRow(separator);
			_letters.insert(yearStr, separator);
			return separator;
		}
		break;
	}
	// Other types of hierarchy, separators are built from letters
	default:
		QString c;
		if (node->data(Miam::DF_CustomDisplayText).toString().isEmpty()) {
			c = node->text().left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		} else {
			QString reorderedText = node->data(Miam::DF_CustomDisplayText).toString();
			c = reorderedText.left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		}
		QString letter;
		bool topLevelLetter = false;
		if (c.contains(QRegExp("\\w"))) {
			letter = c;
		} else {
			letter = tr("Various");
			topLevelLetter = true;
		}
		if (_letters.contains(letter)) {
			return _letters.value(letter);
		} else {
			SeparatorItem *separator = new SeparatorItem(letter);
			if (topLevelLetter) {
				separator->setData("0", Miam::DF_NormalizedString);
			} else {
				separator->setData(letter.toLower(), Miam::DF_NormalizedString);
			}
			invisibleRootItem()->appendRow(separator);
			_letters.insert(letter, separator);
			return separator;
		}
	}
}

void LibraryItemModel::cleanDanglingNodes()
{
	/// XXX: there's an empty row sometimes caused by extra SeparatorItem
	this->rebuildSeparators();
}

/** Recursively remove node and its parent if the latter has no more children. */
void LibraryItemModel::removeNode(const QModelIndex &node)
{
	QModelIndex parent = node.parent();
	this->removeRow(node.row(), parent);
	if (!hasChildren(parent)) {
		this->removeNode(parent);
	}
}

/** Find and insert a node in the hierarchy of items. */
void LibraryItemModel::insertNode(GenericDAO *node)
{
	if (_hash.contains(node->hash())) {
		//qDebug() << "node exists, returning!" << node->title();
		return;
	}

	QStandardItem *nodeItem = NULL;
	if (TrackDAO *dao = qobject_cast<TrackDAO*>(node)) {
		//qDebug() << Q_FUNC_INFO << dao->uri();
		nodeItem = new TrackItem(dao);
		if (_tracks.contains(dao->uri())) {
			QStandardItem *rowToDelete = _tracks.value(dao->uri());
			// Clean unused nodes
			this->removeNode(rowToDelete->index());
		}
		_tracks.insert(dao->uri(), nodeItem);
	} else if (AlbumDAO *dao = qobject_cast<AlbumDAO*>(node)) {
		//qDebug() << Q_FUNC_INFO << "AlbumDAO cover" << dao << dao->cover();
		AlbumItem *album = static_cast<AlbumItem*>(_hash.value(dao->hash()));
		if (album) {
			nodeItem = album;
			//qDebug() << Q_FUNC_INFO << "AlbumItem exists" << album->coverPath();
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
	} else {
		invisibleRootItem()->appendRow(nodeItem);
		if (SeparatorItem *separator = this->insertSeparator(nodeItem)) {
			_topLevelItems.insert(separator, nodeItem->index());
		}
	}
	_hash.insert(node->hash(), nodeItem);
}

void LibraryItemModel::updateNode(GenericDAO *node)
{
	//qDebug() << Q_FUNC_INFO << node->title();
	uint h = node->hash();
	/// Why do I have to update asynchronously this node? Can I just not fill all the information in the Database class?
	if (AlbumItem *album = static_cast<AlbumItem*>(_hash.value(h))) {
		AlbumDAO *dao = qobject_cast<AlbumDAO*>(node);
		album->setData(dao->year(), Miam::DF_Year);
		album->setData(dao->cover(), Miam::DF_CoverPath);
		album->setData(dao->icon(), Miam::DF_IconPath);
		album->setData(!dao->icon().isEmpty(), Miam::DF_IsRemote);
	} else if (TrackItem *track = static_cast<TrackItem*>(_hash.value(h))) {
		TrackDAO *dao = qobject_cast<TrackDAO*>(node);
		qDebug() << Q_FUNC_INFO << dao << track;
	} else if (ArtistItem *artist = static_cast<ArtistItem*>(_hash.value(h))) {
		ArtistDAO *dao = qobject_cast<ArtistDAO*>(node);
		qDebug() << Q_FUNC_INFO << dao << artist;
	}
}
