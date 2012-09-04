#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

#include "library/libraryitem.h"

class TreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit TreeView(QWidget *parent = 0);

	static QString absFilePath(const QModelIndex &index);

protected:
	/** Explore items to count leaves (tracks). */
	virtual int countAll(const QModelIndexList &indexes) const = 0;

	/** Scan nodes and its subitems before dispatching tracks to a specific widget (playlist or tageditor). */
	virtual void findAll(const QPersistentModelIndex &index, QMap<QString, QPersistentModelIndex> &indexes) = 0;

private:
	int beforeSending(const QString &target, QMap<QString, QPersistentModelIndex> &indexes);

protected slots:
	/** Send folders or tracks to the tag editor. */
	void openTagEditor();

	/** Send folders or tracks to the current playlist. */
	void sendToCurrentPlaylist();

signals:
	/** Add a track to the current playlist. */
	void sendToPlaylist(const QList<QPersistentModelIndex> &);

	/** Add a track to the tag editor. */
	void sendToTagEditor(QList<QPersistentModelIndex>);

	void setTagEditorVisible(bool);
};

#endif // TREEVIEW_H
