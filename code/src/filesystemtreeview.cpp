#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QMessageBox>
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
	theIndex = fileSystemModel->setRootPath(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
	this->setRootIndex(theIndex);

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	this->setColumnHidden(1, true);
	this->setColumnHidden(3, true);
	this->header()->setResizeMode(QHeaderView::ResizeToContents);

	properties = new QMenu(this);
	toPlaylist = tr("Add \"%1\" to playlist");
	toLibrary = tr("Add \"%1\" to library");
	toTagEditor = tr("Send \"%1\" to the tag editor");
}

void FileSystemTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QFileInfo fileInfo = fileSystemModel->fileInfo(index);
	properties->clear();

	// Always add the possibility for one to send a folder or a track to the current playlist
	QAction *actionAddToPlayList = new QAction(toPlaylist.arg(fileInfo.baseName()), properties);
	connect(actionAddToPlayList, SIGNAL(triggered()), this, SLOT(addItemsToPlayList()));
	properties->addAction(actionAddToPlayList);

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

/** Reimplemented with a QDirIterator to gather informations about tracks. */
void FileSystemTreeView::findAllAndDispatch(const QModelIndex &index, bool toPlaylist)
{
	QDirIterator dirIterator(fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
	int files = 0;
	while (dirIterator.hasNext()) {
		dirIterator.next();
		files++;
	}

	int ret = QMessageBox::Ok;
	if (files > 300) {
		QMessageBox msgBox;
		QString totalFiles = tr("There are more than 300 files to add to the playlist (%1 to add).");
		msgBox.setText(totalFiles.arg(files));
		msgBox.setInformativeText(tr("Are you sure you want to continue? This might take some time."));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Ok);
		ret = msgBox.exec();
	}
	if (ret == QMessageBox::Ok) {
		QDirIterator dirIteratorOK(fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
		if (!toPlaylist) {
			emit setTagEditorVisible(true);
			emit aboutToBeSent();
		}
		while (dirIteratorOK.hasNext()) {
			QString entry = dirIteratorOK.next();
			QFileInfo fileInfo(entry);
			if (fileInfo.isFile() && FileHelper::suffixes().contains(fileInfo.suffix())) {
				// Dispatch items
				if (toPlaylist) {
					emit sendToPlaylist(QPersistentModelIndex(fileSystemModel->index(entry)));
				} else {
					emit sendToTagEditor(QPersistentModelIndex(fileSystemModel->index(entry)));
				}
			}
		}
		if (!toPlaylist) {
			emit finishedToBeSent();
		}
	}
}

/** Send one folder to the existing music locations. */
void FileSystemTreeView::addFolderToLibrary()
{
	QFileSystemModel *standardItemModel = qobject_cast<QFileSystemModel*>(model());
	QString absFilePath = standardItemModel->fileInfo(this->currentIndex()).absoluteFilePath();
	emit aboutToAddMusicLocation(absFilePath);
}

/** Send folders or tracks to the current playlist. */
void FileSystemTreeView::addItemsToPlayList()
{
	this->findAllAndDispatch(this->currentIndex(), true);
}
