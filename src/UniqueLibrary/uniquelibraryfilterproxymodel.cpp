#include "uniquelibraryfilterproxymodel.h"

#include <QStandardItemModel>
#include <separatoritem.h>
#include <model/sqldatabase.h>
#include <QSqlQuery>

#include <QtDebug>

UniqueLibraryFilterProxyModel::UniqueLibraryFilterProxyModel(QObject *parent)
	: MiamSortFilterProxyModel(parent)
	, _model(nullptr)
{}

/** Redefined from QSortFilterProxyModel. */
void UniqueLibraryFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	MiamSortFilterProxyModel::setSourceModel(sourceModel);
	_model = qobject_cast<QStandardItemModel*>(sourceModel);
}

/** Redefined from MiamSortFilterProxyModel. */
bool UniqueLibraryFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QStandardItem *item = _model->itemFromIndex(_model->index(sourceRow, 1, sourceParent));
	if (!item) {
		return false;
	}
	bool result = false;
	switch (item->type()) {
	case Miam::IT_Artist:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else {
			SqlDatabase db;
			db.init();
			QSqlQuery getArtist(db);
			getArtist.setForwardOnly(true);
			getArtist.prepare("SELECT * FROM tracks WHERE title LIKE ? AND artistId = ?");
			getArtist.addBindValue("%" + filterRegExp().pattern() + "%");
			getArtist.addBindValue(item->data(Miam::DF_ID).toUInt());
			result = getArtist.exec() && getArtist.next();
		}
		break;
	case Miam::IT_Album:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else if (filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1) {
			result = true;
		} else {
			SqlDatabase db;
			db.init();
			QSqlQuery getAlbum(db);
			getAlbum.setForwardOnly(true);
			getAlbum.prepare("SELECT * FROM tracks WHERE title LIKE ? AND albumId = ?");
			getAlbum.addBindValue("%" + filterRegExp().pattern() + "%");
			getAlbum.addBindValue(item->data(Miam::DF_ID).toUInt());
			result = getAlbum.exec() && getAlbum.next();
		}
		break;
	case Miam::IT_Disc:
		if (filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1) {
			result = true;
		} else {
			SqlDatabase db;
			db.init();
			QSqlQuery getDiscAlbum(db);
			getDiscAlbum.setForwardOnly(true);
			getDiscAlbum.prepare("SELECT * FROM tracks WHERE disc > 0 AND title LIKE ? AND albumId = ?");
			getDiscAlbum.addBindValue("%" + filterRegExp().pattern() + "%");
			getDiscAlbum.addBindValue(item->data(Miam::DF_ID).toUInt());
			result = getDiscAlbum.exec() && getDiscAlbum.next();
		}
		break;
	case Miam::IT_Track:
		if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else {
			result = filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1 ||
					filterRegExp().indexIn(item->data(Miam::DF_Album).toString()) != -1;
		}
		break;
	case Miam::IT_Separator:
		for (QModelIndex index : _topLevelItems.values(static_cast<SeparatorItem*>(item))) {
			if (filterAcceptsRow(index.row(), sourceParent)) {
				result = true;
			}
		}
		break;
	default:
		break;
	}
	return result;
}
