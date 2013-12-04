#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

#include <model/libraryitem.h>
#include "playlists/playlist.h"

/**
 * @brief The TreeView class is the base class for displaying trees in the player.
 */
class TreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit TreeView(QWidget *parent = 0);

protected:
	/** Explore items to count leaves (tracks). */
	virtual int countAll(const QModelIndexList &indexes) const = 0;

	/** Scan nodes and its subitems before dispatching tracks to a specific widget (playlist or tageditor). */
	virtual void findAll(const QPersistentModelIndex &index, QStringList &tracks) = 0;

private:
	/** Alerts the user if there's too many tracks to add. */
	int beforeSending(const QString &target, QStringList &tracks);

public slots:
	/** Sends folders or tracks to the end of a playlist. */
	void appendToPlaylist() { this->insertToPlaylist(-1); }

	/** Sends folders or tracks to a specific position in a playlist. */
	void insertToPlaylist(int rowIndex);

	/** Sends folders or tracks to the tag editor. */
	void openTagEditor();

signals:
	/** Adds tracks to the current playlist at a specific position. */
	void aboutToInsertToPlaylist(int rowIndex, const QStringList &tracks);

	/** Adds tracks to the tag editor. */
	void sendToTagEditor(const QStringList &tracks);

	void setTagEditorVisible(bool);
};

#endif // TREEVIEW_H
