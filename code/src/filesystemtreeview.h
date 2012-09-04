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
	explicit FileSystemTreeView(QWidget *parent = 0);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	/** Reimplemented with a QDirIterator to quick count tracks. */
	int countAll(const QModelIndexList &indexes) const;

	/** Reimplemented with a QDirIterator to gather informations about tracks. */
	void findAll(const QPersistentModelIndex &index, QMap<QString, QPersistentModelIndex> &indexes);

private slots:
	/** Send one folder to the existing music locations. */
	void addFolderToLibrary();

signals:
	/** Append the selected folder to the existing music locations. */
	void aboutToAddMusicLocation(const QString &);
};

#endif // FILESYSTEMTREEVIEW_H
