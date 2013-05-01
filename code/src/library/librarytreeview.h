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
	//MusicSearchEngine *musicSearchEngine;
	QPoint currentPos;
	QMenu *properties;

public:
	LibraryTreeView(QWidget *parent = 0);

	/** Small function for translating the QMenu exclusively. */
	void retranslateUi();

	//inline MusicSearchEngine * searchEngine() { return musicSearchEngine; }

	QSize sizeInt() const;

protected:
	/** Redefined to display a small context menu in the view. */
	void contextMenuEvent(QContextMenuEvent *event);

	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	void mouseDoubleClickEvent(QMouseEvent *event);

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	int countAll(const QModelIndexList &indexes) const;

	/** Reimplemented. */
	void findAll(const QPersistentModelIndex &index, QMap<QString, QPersistentModelIndex> &indexes);

public slots:
	/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
	void beginPopulateTree(bool = false);

	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Rebuild a subset of the tree. */
	void rebuild(QList<QPersistentModelIndex> indexes);

    /**  Layout the library at runtime when one is changing the size in options. */
    void setCoverSize(int);

private slots:
	void endPopulateTree();

	//test
	void expandTreeView(const QModelIndex &index);

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
