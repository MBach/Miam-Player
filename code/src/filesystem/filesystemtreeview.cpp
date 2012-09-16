#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QStandardItemModel>

#include "filehelper.h"

#include <QtDebug>

FileSystemTreeView::FileSystemTreeView(QWidget *parent) :
	TreeView(parent)
{
	fileSystemModel = new QFileSystemModel(this);
	fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

	QStringList filters;
	foreach (QString suffix, FileHelper::suffixes()) {
		filters.append("*." + suffix);
	}
	fileSystemModel->setNameFilters(filters);
	this->setModel(fileSystemModel);

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	this->setColumnHidden(1, true);
	this->setColumnHidden(3, true);
	this->header()->setResizeMode(QHeaderView::ResizeToContents);

	properties = new QMenu(this);
	toPlaylist = tr("Add \"%1\" to playlist");
	toLibrary = tr("Add \"%1\" to library");
	toTagEditor = tr("Send \"%1\" to the tag editor");

	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(convertToFolder(QModelIndex)));
}

/** Reimplemented to display up to 3 actions. */
void FileSystemTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QFileInfo fileInfo = fileSystemModel->fileInfo(index);
	properties->clear();

	// Always add the possibility for one to send a folder or a track to the current playlist
	QAction *actionSendToCurrentPlaylist = new QAction(toPlaylist.arg(fileInfo.baseName()), properties);
	connect(actionSendToCurrentPlaylist, SIGNAL(triggered()), this, SLOT(sendToPlaylist()));
	properties->addAction(actionSendToCurrentPlaylist);

	// Same thing for the tag editor
	QAction *actionSendToTagEditor = new QAction(toTagEditor.arg(fileInfo.baseName()), properties);
	connect(actionSendToTagEditor, SIGNAL(triggered()), this, SLOT(openTagEditor()));
	properties->addAction(actionSendToTagEditor);

	// But restricts for the library. It is not wished to add single file as placeholder
	if (fileInfo.isDir()) {
		QAction *actionAddToLibrary = new QAction(toLibrary.arg(fileInfo.baseName()), properties);
		connect(actionAddToLibrary, SIGNAL(triggered()), this, SLOT(addFolderToLibrary()));
		properties->addAction(actionAddToLibrary);
	}
	properties->exec(event->globalPos());
}

/** Reimplemented with a QDirIterator to quick count tracks. */
int FileSystemTreeView::countAll(const QModelIndexList &indexes) const
{
	int files = 0;
	foreach (QModelIndex index, indexes) {
		QDirIterator dirIterator(fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			if (QFileInfo(dirIterator.next()).isFile()) {
				files++;
			}
		}
	}
	return files;
}

/** Reimplemented with a QDirIterator to gather informations about tracks. */
void FileSystemTreeView::findAll(const QPersistentModelIndex &index, QMap<QString, QPersistentModelIndex> &indexes)
{
	QDirIterator dirIterator(fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
	while (dirIterator.hasNext()) {
		QString entry = dirIterator.next();
		QFileInfo fileInfo(entry);
		if (fileInfo.isFile() && FileHelper::suffixes().contains(fileInfo.suffix())) {
			indexes.insert(fileInfo.absoluteFilePath(), fileSystemModel->index(entry));
		}
	}
}

/** Reload tree when the path has changed in the address bar. */
void FileSystemTreeView::reloadWithNewPath(const QString &path)
{
	theIndex = fileSystemModel->setRootPath(path);
	this->setRootIndex(theIndex);
	this->collapseAll();
	this->update(theIndex);
}

/** Send one folder to the existing music locations. */
void FileSystemTreeView::addFolderToLibrary()
{
	QFileSystemModel *standardItemModel = qobject_cast<QFileSystemModel*>(model());
	QString absFilePath = standardItemModel->fileInfo(this->currentIndex()).absoluteFilePath();
	emit aboutToAddMusicLocation(absFilePath);
}

/** Get the folder which is the target of one's double-click. */
void FileSystemTreeView::convertToFolder(const QModelIndex &index)
{
	QFileInfo fileInfo = fileSystemModel->fileInfo(index);
	if (fileInfo.isDir()) {
		emit folderChanged(fileSystemModel->filePath(index));
	}
}
