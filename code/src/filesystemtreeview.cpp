#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QStandardItemModel>

#include "filehelper.h"

#include <QtDebug>

FileSystemTreeView::FileSystemTreeView(QWidget *parent) :
	QTreeView(parent)
{
	QFileSystemModel *fileSystemModel = new QFileSystemModel(this);
	fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

	QStringList filters;
	foreach (QString suffix, FileHelper::suffixes()) {
		filters.append("*." + suffix);
	}
	fileSystemModel->setNameFilters(filters);

	this->setModel(fileSystemModel);
	this->setRootIndex(fileSystemModel->setRootPath(QDesktopServices::storageLocation(QDesktopServices::MusicLocation)));

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	this->setColumnHidden(1, true);
	this->setColumnHidden(3, true);
	this->header()->setResizeMode(QHeaderView::ResizeToContents);

	properties = new QMenu(this);
	addToPlaylist = tr("Add \"%1\" to playlist");
	addToLibrary = tr("Add \"%1\" to library");
}

void FileSystemTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QFileSystemModel *standardItemModel = qobject_cast<QFileSystemModel*>(model());
	QFileInfo fileInfo = standardItemModel->fileInfo(index);
	properties->clear();

	// Always add the possibility for one to send a folder or a track to the current playlist
	QAction *actionAddToPlayList = new QAction(addToPlaylist.arg(fileInfo.baseName()), properties);
	connect(actionAddToPlayList, SIGNAL(triggered()), this, SLOT(addToPlayList()));
	properties->addAction(actionAddToPlayList);

	// But restricts for the library. It is not wished to add single file as placeholder
	if (fileInfo.isDir()) {
		QAction *actionAddToLibrary = new QAction(addToLibrary.arg(fileInfo.baseName()), properties);
		connect(actionAddToLibrary, SIGNAL(triggered()), this, SLOT(addFolderToLibrary()));
		properties->addAction(actionAddToLibrary);
	}
	properties->exec(event->globalPos());
}

/** Send one folder to the existing music locations. */
void FileSystemTreeView::addFolderToLibrary()
{
	QFileSystemModel *standardItemModel = qobject_cast<QFileSystemModel*>(model());
	QString absFilePath = standardItemModel->fileInfo(this->currentIndex()).absoluteFilePath();
	emit aboutToAddMusicLocation(absFilePath);
}

void FileSystemTreeView::addToPlayList()
{
	qDebug() << "TODO";
}
