#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

#include "library/libraryitem.h"
#include "playlist.h"

/**
 * @brief The TreeView class is the base class for displaying trees in the player.
 */
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

public slots:
	/** Send folders or tracks to a playlist. */
	void sendToPlaylist(Playlist *playlist = 0, int row = -1);

protected slots:
	/** Send folders or tracks to the tag editor. */
	void openTagEditor();

signals:
	/** Add tracks to the a playlist. */
	void aboutToSendToPlaylist(const QList<QPersistentModelIndex> &, Playlist*, int);

	/** Add tracks to the tag editor. */
	void sendToTagEditor(QList<QPersistentModelIndex>);

	void setTagEditorVisible(bool);
};

#endif // TREEVIEW_H
