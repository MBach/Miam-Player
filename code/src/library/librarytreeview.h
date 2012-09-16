#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <QMenu>
#include <QSortFilterProxyModel>

#include "librarymodel.h"
#include "circleprogressbar.h"
#include "musicsearchengine.h"
#include "libraryfilterproxymodel.h"
#include "treeview.h"

class LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	LibraryModel *libraryModel;
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	MusicSearchEngine *musicSearchEngine;
	QPoint currentPos;
	QMenu *properties;

public:
	LibraryTreeView(QWidget *parent = 0);

	/** Small function for translating the QMenu exclusively. */
	void retranslateUi();

	inline MusicSearchEngine * searchEngine() { return musicSearchEngine; }

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

	void sendSingleItemToPlaylist(const QModelIndex &/*index*/);

private slots:
	/** Tell the view to create specific delegate for the current row. */
	void addNodeToTree(LibraryItem *libraryItem);

	void endPopulateTree();

	//test
	void expandTreeView(const QModelIndex &index);

	/**  Layout the library at runtime when one is changing the size in options. */
	void setCoverSize(int);

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	/** When covers are enabled, changes their size. */
	void sizeOfCoversChanged(int);
};

#endif // LIBRARYTREEVIEW_H
