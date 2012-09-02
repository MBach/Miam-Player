#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QTreeView>

class FileSystemTreeView : public QTreeView
{
	Q_OBJECT
private:
	QString addToLibrary;
	QString addToPlaylist;
	QMenu *properties;

public:
	FileSystemTreeView(QWidget *parent = 0);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private slots:
	/** Send one folder to the existing music locations. */
	void addFolderToLibrary();

	void addToPlayList();

signals:
	void aboutToAddMusicLocation(const QString &);
};

#endif // FILESYSTEMTREEVIEW_H
