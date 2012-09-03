#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "treeview.h"

#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QMenu>

class FileSystemTreeView : public TreeView
{
	Q_OBJECT
private:
	QString toLibrary;
	QString toPlaylist;
	QString toTagEditor;
	QMenu *properties;
	QModelIndex theIndex;
	QFileSystemModel *fileSystemModel;

public:
	FileSystemTreeView(QWidget *parent = 0);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	/** Reimplemented with a QDirIterator to gather informations about tracks. */
	void findAllAndDispatch(const QModelIndex &index, bool toPlaylist = true);

private slots:
	/** Send one folder to the existing music locations. */
	void addFolderToLibrary();

	/** Send folders or tracks to the current playlist. */
	void addItemsToPlayList();

signals:
	/** Append the selected folder to the existing music locations. */
	void aboutToAddMusicLocation(const QString &);
};

#endif // FILESYSTEMTREEVIEW_H
