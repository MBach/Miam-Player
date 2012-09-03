#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

#include "library/libraryitem.h"

class TreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit TreeView(QWidget *parent = 0);

	static QString absFilePath(const QPersistentModelIndex &index);

protected slots:
	/** Recursively scan one node and its subitems before dispatching tracks to a specific widget (playlist or tageditor).*/
	virtual void findAllAndDispatch(const QModelIndex &index, bool toPlaylist = true) = 0;

	void openTagEditor();

signals:
	/** Tracks are about to be sent to a playlist or a tag editor. */
	void aboutToBeSent();

	/** Tracks are completely sent to a playlist or a tag editor. */
	void finishedToBeSent();

	/** Add a track to the current playlist. */
	void sendToPlaylist(QModelIndex);

	/** Add a track to the tag editor. */
	void sendToTagEditor(QModelIndex);

	void setTagEditorVisible(bool);
};

#endif // TREEVIEW_H
