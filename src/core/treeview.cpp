#include "treeview.h"

#include "settings.h"

#include <QDrag>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>

#include <QtDebug>

TreeView::TreeView(QWidget *parent)
	: QTreeView(parent)
	, SelectedTracksModel()
{
	this->setAttribute(Qt::WA_MacShowFocusRect, false);
}

TreeView::~TreeView()
{}

QList<QUrl> TreeView::selectedTracks()
{
	QList<QUrl> list;
	for (QModelIndex index : this->selectionModel()->selectedRows()) {
		this->findAll(index, &list);
	}
	return list;
}

bool TreeView::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->modifiers().testFlag(Qt::NoModifier)) {
			// If one has assigned a simple key like 'N' to 'Skip Forward' we don't actually want to skip the track
			if (65 <= keyEvent->key() && keyEvent->key() <= 90) {
				// We don't want this event to be propagated
				keyEvent->accept();
				return true;
			}
		} else {
			keyEvent->ignore();
			return false;
		}
	}
	return QTreeView::eventFilter(obj, event);
}

void TreeView::startDrag(Qt::DropActions)
{
	QByteArray itemData;
	QList<QUrl> urls = this->selectedTracks();

	for (int i = 0; i < urls.size(); i++) {
		QUrl u = urls.at(i);
		itemData.append(u.toEncoded());
		if (i + 1 < urls.size()) {
			itemData.append('|');
		}
	}
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
void TreeView::appendToPlaylist()
{
	QList<QUrl> tracks;
	if (this->beforeSending(tr("playlist"), &tracks) == QMessageBox::Ok) {
		emit aboutToInsertToPlaylist(-1, tracks);
	}
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QList<QUrl> tracks;
	if (this->beforeSending(tr("tag editor"), &tracks) == QMessageBox::Ok) {
		emit aboutToSendToTagEditor(tracks);
	}
}
