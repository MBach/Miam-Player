#include "treeview.h"

#include "settings.h"

#include <QFileSystemModel>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent)
{
}

QString TreeView::absFilePath(const QPersistentModelIndex &index)
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

/// TEST
void TreeView::openTagEditor()
{
	/// Can those signals be factorised?
	emit setTagEditorVisible(true);
	emit aboutToBeSent();

	// Feed with new indexes
	QModelIndexList indexes = this->selectedIndexes();
	foreach (QModelIndex index, indexes) {
		this->findAllAndDispatch(index, false);
	}
	emit finishedToBeSent();
}
