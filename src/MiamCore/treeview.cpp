#include "treeview.h"

//#include "mainwindow.h"
#include "settings.h"

#include <QDrag>
#include <QMessageBox>
#include <QMimeData>

#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent), SelectedTracksModel()
{
	this->setAttribute(Qt::WA_MacShowFocusRect, false);
}

QList<QUrl> TreeView::selectedTracks()
{
	QList<QUrl> list;
	_cacheSelectedIndexes.clear();
	for (QModelIndex index : this->selectionModel()->selectedIndexes()) {
		_cacheSelectedIndexes << index;
		this->findAll(index, &list);
	}
	return list;
}

void TreeView::startDrag(Qt::DropActions)
{
	QByteArray itemData;
	QMimeData *mimeData = new QMimeData;
	mimeData->setData("treeview/x-treeview-item", itemData);
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
}

/** Alerts the user if there's too many tracks to add. */
QMessageBox::StandardButton TreeView::beforeSending(const QString &target, QList<QUrl> *tracks)
{
	// Quick count tracks before anything else
	int count = this->countAll(selectedIndexes());

	QMessageBox::StandardButton ret = Miam::showWarning(target, count);

	if (ret == QMessageBox::Ok) {
		// Gather all items (pure virtual call, this function must be reimplemented in subclasses: custom tree, file system, etc.)
		for (QModelIndex index : selectedIndexes()) {
			this->findAll(index, tracks);
		}
	}
	return ret;
}

/** Send folders or tracks to a specific position in a playlist. */
void TreeView::insertToPlaylist(int rowIndex)
{
	QList<QUrl> tracks;
	if (this->beforeSending(tr("playlist"), &tracks) == QMessageBox::Ok) {
		emit aboutToInsertToPlaylist(rowIndex, tracks);
	}
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QList<QUrl> tracks;
	if (this->beforeSending(tr("tag editor"), &tracks) == QMessageBox::Ok) {
		emit sendToTagEditor(selectedIndexes(), tracks);
	}
}
