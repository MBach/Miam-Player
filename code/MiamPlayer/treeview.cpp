#include "treeview.h"

#include "settings.h"

#include <QFileSystemModel>
#include <QMessageBox>

#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent)
{
}

/** Alerts the user if there's too many tracks to add. */
int TreeView::beforeSending(const QString &target, QStringList &tracks)
{
	// Quick count tracks before anything else
	int count = this->countAll(selectedIndexes());
	qDebug() << "tracks to add" << count;

	int ret = QMessageBox::Ok;
    /// XXX: extract magic number (to where?)
	if (count > 300) {
		QMessageBox msgBox;
		QString totalFiles = tr("There are more than 300 files to add to the %1 (%2 to add).");
		msgBox.setText(totalFiles.arg(target).arg(count));
		msgBox.setInformativeText(tr("Are you sure you want to continue? This might take some time."));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Ok);
		ret = msgBox.exec();
	}

	if (ret == QMessageBox::Ok) {
		// Gather all items (pure virtual call, this function must be reimplemented in subclasses: custom tree, file system, etc.)
		foreach (QPersistentModelIndex index, selectedIndexes()) {
			this->findAll(index, tracks);
		}
	}
	return ret;
}

/** Send folders or tracks to a specific position in a playlist. */
void TreeView::insertToPlaylist(int rowIndex)
{
	QStringList tracks;
	if (this->beforeSending(tr("playlist"), tracks) == QMessageBox::Ok) {
		emit aboutToInsertToPlaylist(rowIndex, tracks);
	}
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QStringList tracks;
	if (this->beforeSending(tr("tag editor"), tracks) == QMessageBox::Ok) {
		emit setTagEditorVisible(true);
		emit sendToTagEditor(tracks);
	}
}
