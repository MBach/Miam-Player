#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "../treeview.h"

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
	/** Reimplemented to display up to 3 actions. */
	void contextMenuEvent(QContextMenuEvent *event);

private:
	/** Reimplemented with a QDirIterator to quick count tracks. */
	int countAll(const QModelIndexList &indexes) const;

	/** Reimplemented with a QDirIterator to gather informations about tracks. */
	void findAll(const QPersistentModelIndex &index, QStringList &tracks);

public slots:
	/** Reload tree when the path has changed in the address bar. */
	void reloadWithNewPath(const QDir &path);

private slots:
	/** Get the folder which is the target of one's double-click. */
	void convertIndex(const QModelIndex &index);

signals:
	/** Append the selected folder to the existing music locations. */
	void aboutToAddMusicLocations(const QList<QDir> &);

	void folderChanged(const QString &);
};

#endif // FILESYSTEMTREEVIEW_H
