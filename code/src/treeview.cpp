#include "treeview.h"

#include "settings.h"

#include <QFileSystemModel>
#include <QMessageBox>

#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent)
{
}

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

int TreeView::beforeSending(const QString &target, QMap<QString, QModelIndex> &indexes)
{
	// Quick count tracks before anything else
	int count = this->countAll(selectedIndexes());

	int ret = QMessageBox::Ok;
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
		// Gather all items (pure virtual call reimplemented in subclasses)
		foreach (QModelIndex index, selectedIndexes()) {
			this->findAll(index, indexes);
		}
	}
	return ret;
}

/** Send folders or tracks to the tag editor. */
void TreeView::openTagEditor()
{
	QMap<QString, QModelIndex> indexes;
	if (this->beforeSending(tr("tag editor"), indexes) == QMessageBox::Ok) {
		emit setTagEditorVisible(true);
		emit sendToTagEditor(indexes.values());
	}
}

/** Send folders or tracks to the current playlist. */
void TreeView::sendToCurrentPlaylist()
{
	QMap<QString, QModelIndex> indexes;
	if (this->beforeSending(tr("playlist"), indexes) == QMessageBox::Ok) {
		emit sendToPlaylist(indexes.values());
	}
}
