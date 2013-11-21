#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <QMenu>
#include <QSortFilterProxyModel>

#include "librarymodel.h"
#include "circleprogressbar.h"
#include "musicsearchengine.h"
#include "libraryfilterproxymodel.h"
#include "treeview.h"

/**
 * @brief The LibraryTreeView class is displaying tracks in a tree, where items are sorted in Artists > Albums > Tracks.
 */
class LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	LibraryModel *libraryModel;
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	QPoint currentPos;
	QMenu *properties;

public:
	explicit LibraryTreeView(QWidget *parent = 0);

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	/** Reimplemented. */
	virtual void findAll(const QPersistentModelIndex &index, QStringList &tracks);

public slots:
	/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
	void beginPopulateTree(bool = false);

	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Rebuild a subset of the tree. */
	void rebuild(QList<QPersistentModelIndex> indexes);

private slots:
	void endPopulateTree();

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
