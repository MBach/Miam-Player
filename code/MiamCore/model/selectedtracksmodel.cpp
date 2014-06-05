#include "selectedtracksmodel.h"

#include <QtDebug>

#include "../MiamPlayer/library/librarytreeview.h"

SelectedTracksModel::SelectedTracksModel(QItemSelectionModel *itemSelectionModel) :
	QObject(itemSelectionModel), _itemSelectionModel(itemSelectionModel)
{
}

QStringList SelectedTracksModel::selectedTracks() const
{
	QStringList tracks;
	if (!_itemSelectionModel) {
		return tracks;
	}

	//const LibraryTreeView *treeView = qobject_cast<const LibraryTreeView*>(_itemSelectionModel->model());
	/*if (qobject_cast<const TreeView*>(_itemSelectionModel->model())) {

		treeView->findAll(_itemSelectionModel->selectedIndexes(), tracks);
	}*/
	qDebug() << tracks;

	/*foreach (QModelIndex index, _itemSelectionModel->selectedIndexes()) {
		if (index.column() == 0) {
			QVariant v = index.data(Qt::UserRole + 1);
			if (v.isValid()) {
				qDebug() << "type" << v.toInt();
				/// FIXME: Magic numbers from LibraryTreeView class
				/// Problem: LibraryTreeView is in MiamPlayer, not MiamCore, so it's not supposed to be accessible
				/// It's not relevant to create an enumeration in MiamCore just for a specific view
				switch (v.toInt()) {
				case 0: {
					qDebug() << "fetch covers for this artist" << index.data();
					artists.append(index.data().toString());
					break;
				}
				case 1: {
					QString album = index.data().toString();
					qDebug() << "fetch cover for this album" << album;
					qDebug() << "parent is valid?" << index.parent().isValid();
					if (index.parent().isValid()) {
						qDebug() << index.parent().data().toString();
					} else {
						qDebug() << "parent is invalid!";
						qDebug() << "child is valid?" << index.child(0, 0).isValid();
						qDebug() << "children" << index.model()->rowCount(index);
					}
					break;
				}
				default:
					break;
				}
			} else if (index.parent().isValid() && index.child(0, 0).isValid()) {
				// Table Model (like TagEditor)
				//index.sibling()
			}
		}
	}*/
	return tracks;
}
