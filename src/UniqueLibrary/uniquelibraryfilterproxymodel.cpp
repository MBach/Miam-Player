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
			QSqlQuery getArtist(db);
			getArtist.setForwardOnly(true);
			getArtist.prepare("SELECT * FROM cache WHERE trackTitle LIKE ?");
			getArtist.addBindValue("%" + filterRegExp().pattern() + "%");
			result = getArtist.exec() && getArtist.next();
		}
		break;
	case Miam::IT_Album:
		if (item->text().contains(filterRegExp().pattern(), Qt::CaseInsensitive)) {
			result = true;
		} else {
			SqlDatabase db;
			QSqlQuery getAlbum(db);
			getAlbum.setForwardOnly(true);
			getAlbum.prepare("SELECT * FROM cache WHERE trackTitle LIKE ?");
			getAlbum.addBindValue("%" + filterRegExp().pattern() + "%");
			result = getAlbum.exec() && getAlbum.next();
		}
		break;
	case Miam::IT_Disc:
		if (filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1) {
			result = true;
		} else {
			SqlDatabase db;
			QSqlQuery getDiscAlbum(db);
			getDiscAlbum.setForwardOnly(true);
			getDiscAlbum.prepare("SELECT * FROM cache WHERE disc > 0 AND trackTitle LIKE ?");
			getDiscAlbum.addBindValue("%" + filterRegExp().pattern() + "%");
			result = getDiscAlbum.exec() && getDiscAlbum.next();
		}
		break;
	case Miam::IT_Track:
		/*if (MiamSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
			result = true;
		} else {
			result = filterRegExp().indexIn(item->data(Miam::DF_Artist).toString()) != -1 ||
					filterRegExp().indexIn(item->data(Miam::DF_Album).toString()) != -1;
		}*/
		result = item->text().contains(filterRegExp().pattern(), Qt::CaseInsensitive);
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
