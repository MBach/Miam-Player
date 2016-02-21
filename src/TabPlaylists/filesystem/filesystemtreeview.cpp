#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QStandardItemModel>

#include <styling/miamstyleditemdelegate.h>
#include <filehelper.h>
#include <scrollbar.h>
#include "nofocusitemdelegate.h"

#include <QtDebug>

FileSystemTreeView::FileSystemTreeView(QWidget *parent)
	: TreeView(parent)
	, _properties(new QMenu(this))
	, _fileSystemModel(new QFileSystemModel(this))
{
	this->installEventFilter(this);
	_fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	_fileSystemModel->setNameFilters(FileHelper::suffixes(FileHelper::All, true));
	this->setModel(_fileSystemModel);
	this->setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	this->setColumnHidden(1, true);
	this->setColumnHidden(2, true);
	this->setColumnHidden(3, true);

	this->header()->hide();
	this->setItemDelegate(new MiamStyledItemDelegate(this, false));

	_toPlaylist = tr("Add \"%1\" to playlist");
	_toLibrary = tr("Add \"%1\" to library");
	_toTagEditor = tr("Send \"%1\" to the tag editor");

	connect(this, &FileSystemTreeView::doubleClicked, this, &FileSystemTreeView::convertIndex);
}

/** Reimplemented with a QDirIterator to gather informations about tracks. */
void FileSystemTreeView::findAll(const QModelIndex &index, QList<QUrl> *tracks) const
{
	QFileInfo fileInfo = _fileSystemModel->fileInfo(index);
	QStringList files;
	if (fileInfo.isFile()) {
		files << fileInfo.absoluteFilePath();
	} else {
		qDebug() << Q_FUNC_INFO;
		QDirIterator dirIterator(fileInfo.absoluteFilePath(), QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			QString entry = dirIterator.next();
			QFileInfo fileInfo(entry);
			if (fileInfo.isFile() && FileHelper::suffixes(FileHelper::All).contains(fileInfo.suffix())) {
				files << fileInfo.absoluteFilePath();
			}
		}
	}
	files.sort(Qt::CaseInsensitive);
	files.removeDuplicates();
	for (QString f : files) {
		tracks->append(QUrl::fromLocalFile(f));
	}
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

void FileSystemTreeView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	// Move up in the filesystem
	case Qt::Key_Backspace: {
		QString path = _fileSystemModel->filePath(_theIndex);
		QFileInfo fileInfo(path);
		if (fileInfo.isDir()) {
			QDir d(path);
			if (d.cdUp()) {
				emit folderChanged(d.absolutePath());
			}
		}
		break;
	}
	// Change directory if selected one is a directory, otherwise, send track to playlist
	case Qt::Key_Return:
	case Qt::Key_Enter: {
		if (selectedIndexes().size() == 1) {
			QString path = _fileSystemModel->filePath(selectedIndexes().first());
			QFileInfo fileInfo(path);
			if (fileInfo.isDir()) {
				emit folderChanged(fileInfo.absoluteFilePath());
			} else if (FileHelper::suffixes().contains(fileInfo.suffix())) {
				this->convertIndex(selectedIndexes().first());
			}
		} else {
			for (QModelIndex index : selectedIndexes()) {
				this->convertIndex(index);
			}
		}
		break;
	}
	case Qt::Key_Space: {
		if (selectedIndexes().isEmpty()) {
			this->selectionModel()->select(_fileSystemModel->index(0, 0, _theIndex), QItemSelectionModel::Select);
		}
		break;
	}
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
		TreeView::keyPressEvent(event);
		break;
	default:
		event->isAccepted();
		this->scrollAndHighlight((QChar)event->key());
	}
}

/** Reimplemented with a QDirIterator to quick count tracks. */
int FileSystemTreeView::countAll(const QModelIndexList &indexes) const
{
	int files = 0;
	for (QModelIndex index : indexes) {
		QDirIterator dirIterator(_fileSystemModel->fileInfo(index).absoluteFilePath(), QDirIterator::Subdirectories);
		while (dirIterator.hasNext()) {
			if (QFileInfo(dirIterator.next()).isFile()) {
				files++;
			}
		}
	}
	return files;
}

void FileSystemTreeView::scrollAndHighlight(const QChar &c)
{
	QModelIndexList rows = this->selectionModel()->selectedRows();

	// If there's already an item matching the key which has just been typed
	if (rows.size() == 1 &&	rows.first().data().toString().startsWith(c, Qt::CaseInsensitive)) {
		QModelIndex next = rows.first().sibling(rows.first().row() + 1, 0);
		if (next.isValid() && next.data().toString().startsWith(c, Qt::CaseInsensitive)) {
			this->setCurrentIndex(next);
			this->selectionModel()->select(next, QItemSelectionModel::Current);
		} else {
			// Find first with with letter corresponding to key
			QModelIndex previous = next.sibling(next.row() - 1, 0);
			QModelIndex previous_previous;
			while (previous.isValid() && previous.data().toString().startsWith(c, Qt::CaseInsensitive)) {
				previous_previous = previous.sibling(previous.row() - 1, 0);
				if (previous_previous.isValid()) {
					previous = previous_previous;
				} else {
					break;
				}
			}
			// Move forward one last time
			if (previous_previous.isValid()) {
				previous = previous.sibling(previous.row() + 1, 0);
			}
			this->setCurrentIndex(previous);
			this->selectionModel()->select(previous, QItemSelectionModel::Current);
		}
	} else {
		// No item were selected or not from the key pressed, find the correct one (if exists)
		for (int i = 0; i < _fileSystemModel->rowCount(_theIndex); i++) {
			QModelIndex child = _fileSystemModel->index(i, 0, _theIndex);
			if (child.data().toString().startsWith(c, Qt::CaseInsensitive)) {
				this->scrollTo(child, QAbstractItemView::PositionAtTop);
				this->setCurrentIndex(child);
				this->selectionModel()->select(child, QItemSelectionModel::Current);
				break;
			}
		}
	}
}

/** Reload tree when the path has changed in the address bar. */
void FileSystemTreeView::reloadWithNewPath(const QDir &path)
{
	_theIndex = _fileSystemModel->setRootPath(path.absolutePath());
	this->setRootIndex(_theIndex);
	this->collapseAll();
	this->update(_theIndex);
	this->setFocus();
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
		emit aboutToInsertToPlaylist(-1, { fileInfo.absoluteFilePath() });
	}
}
