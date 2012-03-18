#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

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

public:
	LibraryTreeView(QWidget *parent = 0);

protected:
	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	void mouseDoubleClickEvent(QMouseEvent *event);
	
signals:
	/** Add a track to the current playlist. */
	void sendToPlaylist(const QPersistentModelIndex &);
	void setIcon(bool value);

public slots:
	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Create the tree from a previously saved flat file, or directly from the hard-drive.*/
	void beginPopulateTree(bool = false);

private slots:
	/** Tell the view to create specific delegate for the current row. */
	void addNodeToTree(int);

	/** Check if the current double-clicked item is an Artist, an Album or a Track.*/
	void beforeSendToPlaylist(const QModelIndex &index);

	void readFile(int musicLocationIndex, const QString &qFileName);

	void endPopulateTree();

	//test
	void expandTreeView(const QModelIndex &index);
};

#endif // LIBRARYTREEVIEW_H
