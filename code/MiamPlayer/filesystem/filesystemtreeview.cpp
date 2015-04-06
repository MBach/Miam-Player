#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QStandardItemModel>

#include "filehelper.h"
#include "nofocusitemdelegate.h"
#include "styling/miamstyleditemdelegate.h"

#include <QtDebug>

FileSystemTreeView::FileSystemTreeView(QWidget *parent) :
	TreeView(parent)
{
	_fileSystemModel = new QFileSystemModel(this);
	_fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

	_fileSystemModel->setNameFilters(FileHelper::suffixes(FileHelper::All, true));

	this->setModel(_fileSystemModel);

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	this->setColumnHidden(1, true);
	this->setColumnHidden(2, true);
	this->setColumnHidden(3, true);

	this->header()->hide();
	this->setItemDelegate(new MiamStyledItemDelegate(this, false));

	_properties = new QMenu(this);
	_toPlaylist = tr("Add \"%1\" to playlist");
	_toLibrary = tr("Add \"%1\" to library");
	_toTagEditor = tr("Send \"%1\" to the tag editor");

	connect(this, &FileSystemTreeView::doubleClicked, this, &FileSystemTreeView::convertIndex);
}

/** Reimplemented with a QDirIterator to gather informations about tracks. */
void FileSystemTreeView::findAll(const QModelIndex &index, QStringList &tracks) const
{
	QFileInfo fileInfo = _fileSystemModel->fileInfo(index);
	if (fileInfo.isFile()) {
		tracks << "file://" + fileInfo.absoluteFilePath();
	} else {
		QDirIterator dirIterator(fileInfo.absoluteFilePath(), QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			QString entry = dirIterator.next();
			QFileInfo fileInfo(entry);
			if (fileInfo.isFile() && FileHelper::suffixes(FileHelper::All).contains(fileInfo.suffix())) {
				tracks << "file://" + fileInfo.absoluteFilePath();
			}
		}
	}
	tracks.sort(Qt::CaseInsensitive);
	tracks.removeDuplicates();
}

/** Reimplemented to display up to 3 actions. */
void FileSystemTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex index = this->indexAt(event->pos());
	QFileInfo fileInfo = _fileSystemModel->fileInfo(index);
	_properties->clear();

	// Always add the possibility for one to send a folder or a track to the current playlist
	QAction *actionSendToCurrentPlaylist = new QAction(_toPlaylist.arg(fileInfo.baseName()), _properties);
	connect(actionSendToCurrentPlaylist, &QAction::triggered, this, &TreeView::appendToPlaylist);
	_properties->addAction(actionSendToCurrentPlaylist);

	// Same thing for the tag editor
	QAction *actionSendToTagEditor = new QAction(_toTagEditor.arg(fileInfo.baseName()), _properties);
	connect(actionSendToTagEditor, &QAction::triggered, this, &TreeView::openTagEditor);
	_properties->addAction(actionSendToTagEditor);

	// But restricts for the library. It is not wished to add single file as placeholder
	if (fileInfo.isDir()) {
		QAction *actionAddToLibrary = new QAction(_toLibrary.arg(fileInfo.baseName()), _properties);
		connect(actionAddToLibrary, &QAction::triggered, this, [=]() {
			QList<QDir> dirs = QList<QDir>() << _fileSystemModel->fileInfo(currentIndex()).absoluteFilePath();
			emit aboutToAddMusicLocations(dirs);
		});
		_properties->addAction(actionAddToLibrary);
	}
	_properties->exec(event->globalPos());
}

/** Reimplemented with a QDirIterator to quick count tracks. */
int FileSystemTreeView::countAll(const QModelIndexList &indexes) const
{
	int files = 0;
	foreach (QModelIndex index, indexes) {
		QDirIterator dirIterator(_fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			if (QFileInfo(dirIterator.next()).isFile()) {
				files++;
			}
		}
	}
	return files;
}
/** Reload tree when the path has changed in the address bar. */
void FileSystemTreeView::reloadWithNewPath(const QDir &path)
{
	theIndex = _fileSystemModel->setRootPath(path.absolutePath());
	this->setRootIndex(theIndex);
	this->collapseAll();
	this->update(theIndex);
}

void FileSystemTreeView::updateSelectedTracks()
{
	qDebug() << Q_FUNC_INFO << "not yet implemented";
}

/** Get the folder which is the target of one's double-click. */
void FileSystemTreeView::convertIndex(const QModelIndex &index)
{
	QFileInfo fileInfo = _fileSystemModel->fileInfo(index);
	if (fileInfo.isDir()) {
		emit folderChanged(_fileSystemModel->filePath(index));
	} else {
		QStringList tracks;
		tracks << "file://" + fileInfo.absoluteFilePath();
		emit aboutToInsertToPlaylist(-1, tracks);
	}
}
