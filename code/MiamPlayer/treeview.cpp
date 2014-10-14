#include "treeview.h"

#include "mainwindow.h"
#include "settings.h"

#include <QMessageBox>

#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent), SelectedTracksModel()
{
	this->setAttribute(Qt::WA_MacShowFocusRect, false);
}

QList<TrackDAO> TreeView::selectedTracks()
{
	QList<TrackDAO> list;
	_cacheSelectedIndexes.clear();
	foreach (QModelIndex index, this->selectionModel()->selectedIndexes()) {
		_cacheSelectedIndexes << index;
		this->findAll(index, list);
	}
	return list;
}

/** Alerts the user if there's too many tracks to add. */
QMessageBox::StandardButton TreeView::beforeSending(const QString &target, QList<TrackDAO> &tracks)
{
	// Quick count tracks before anything else
	int count = this->countAll(selectedIndexes());

	QMessageBox::StandardButton ret = MainWindow::showWarning(target, count);

	if (ret == QMessageBox::Ok) {
		// Gather all items (pure virtual call, this function must be reimplemented in subclasses: custom tree, file system, etc.)
		foreach (QModelIndex index, selectedIndexes()) {
			this->findAll(index, tracks);
		}
	}
	return ret;
}

/** Send folders or tracks to a specific position in a playlist. */
void TreeView::insertToPlaylist(int rowIndex)
{
	QList<TrackDAO> tracks;
	if (this->beforeSending(tr("playlist"), tracks) == QMessageBox::Ok) {
		emit aboutToInsertToPlaylist(rowIndex, tracks);
	}
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QList<TrackDAO> tracks;
	if (this->beforeSending(tr("tag editor"), tracks) == QMessageBox::Ok) {
		emit sendToTagEditor(selectedIndexes(), tracks);
	}
}
