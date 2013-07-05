#include "treeview.h"

#include "settings.h"

#include <QFileSystemModel>
#include <QMessageBox>

#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent)
{
}

///XXX Is it really a good design to enumerate all subclasses below? What if I add a 3rd model later?
QString TreeView::absFilePath(const QModelIndex &index)
{
	const QFileSystemModel *fileSystemModel = qobject_cast<const QFileSystemModel*>(index.model());
	if (fileSystemModel) {
		return fileSystemModel->fileInfo(index).absoluteFilePath();
	} else {
		QString path = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
		QString name = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
		return QFileInfo(path + name).absoluteFilePath();
	}
}

/** Alerts the user if there's too many tracks to add. */
int TreeView::beforeSending(const QString &target, QMap<QString, QPersistentModelIndex> &indexes)
{
	// Quick count tracks before anything else
	int count = this->countAll(selectedIndexes());

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
			this->findAll(index, indexes);
		}
	}
	return ret;
}

/** Send folders or tracks to a specific position in a playlist. */
void TreeView::insertToPlaylist(int rowIndex)
{
	QMap<QString, QPersistentModelIndex> indexes;
	if (this->beforeSending(tr("playlist"), indexes) == QMessageBox::Ok) {
		emit aboutToInsertToPlaylist(rowIndex, indexes.values());
	}
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QMap<QString, QPersistentModelIndex> indexes;
	if (this->beforeSending(tr("tag editor"), indexes) == QMessageBox::Ok) {
		emit setTagEditorVisible(true);
		emit sendToTagEditor(indexes.values());
	}
}
