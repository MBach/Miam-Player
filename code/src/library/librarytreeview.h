#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <QMenu>
#include <QTreeView>
#include <QSortFilterProxyModel>

#include "librarymodel.h"
#include "circleprogressbar.h"
#include "musicsearchengine.h"
#include "libraryfilterproxymodel.h"

class LibraryTreeView : public QTreeView
{
	Q_OBJECT

private:
	LibraryModel *libraryModel;
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	QModelIndexList savedStateModelIndexList;
	MusicSearchEngine *musicSearchEngine;
	QPoint currentPos;
	QMenu *properties;

public:
	LibraryTreeView(QWidget *parent = 0);

	/** Small function for translating the QMenu exclusively. */
	void retranslateUi();

protected:
	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	void mouseDoubleClickEvent(QMouseEvent *event);

private:
	void removeNode(QModelIndex index);

signals:
	/** Tracks are about to be sent to a playlist or a tag editor. */
	void aboutToBeSent();

	/** Tracks are completely sent to a playlist or a tag editor. */
	void finishedToBeSent();

	/** Add a track to the current playlist. */
	void sendToPlaylist(const QPersistentModelIndex &);
	void sendToTagEditor(const QPersistentModelIndex &);

	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	/** When covers are enabled, changes their size. */
	void sizeOfCoversChanged(int);

	void setTagEditorVisible(bool);

public slots:
	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
	void beginPopulateTree(bool = false);

	/** Rebuild a subset of the tree. */
	void rebuild(QList<QPersistentModelIndex>);

private slots:
	/** Tell the view to create specific delegate for the current row. */
	void addNodeToTree(LibraryItem *libraryItem);

	/** Recursively scan one node and its subitems before dispatching tracks to a specific widget (playlist or tageditor).*/
	void findAllAndDispatch(const QModelIndex &index, bool toPlaylist = true);

	void endPopulateTree();

	//test
	void expandTreeView(const QModelIndex &index);

	/**  Layout the library at runtime when one is changing the size in options. */
	void setCoverSize(int);

	void openTagEditor();

	void sendToCurrentPlaylist();
	void showContextMenu(QPoint point);
};

#endif // LIBRARYTREEVIEW_H
