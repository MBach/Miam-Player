#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <model/librarysqlmodel.h>
#include <settings.h>

#include "circleprogressbar.h"
#include "libraryfilterproxymodel.h"
#include "treeview.h"
#include "library/libraryorderdialog.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

/**
 * @brief The LibraryTreeView class is displaying tracks in a tree, where items are sorted in Artists > Albums > Tracks.
 */
class LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	LibraryOrderDialog *_lod;
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	QPoint currentPos;
	QMenu *properties;

	QStandardItemModel* _libraryModel;
	LibrarySqlModel *sqlModel;

	Settings::InsertPolicy _currentInsertPolicy;

	QMap<QString, QStandardItem*> _artists;
	QHash<QPair<QStandardItem*, QString>, QStandardItem*> _albums;
	QHash<QPair<QStandardItem*, int>, QStandardItem*> _discNumbers;
	QHash<QString, QStandardItem*> _albums2;
	QHash<QString, QStandardItem*> _albumsAbsPath;
	QHash<QString, QStandardItem*> _artistsAlbums;
	QHash<int, QStandardItem*> _years;
	QSet<QString> _letters;

public:
	explicit LibraryTreeView(QWidget *parent = 0);

	void init(LibrarySqlModel *sql);

	void insertLetter(const QString &letters);

	void insertTrack(const FileHelper &fh);

	inline Settings::InsertPolicy currentInsertPolicy() const { return _currentInsertPolicy; }

	inline void setInsertPolicy(Settings::InsertPolicy policy) { _currentInsertPolicy = policy; }

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
	void reset();

	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Rebuild a subset of the tree. */
	//void rebuild(QList<QPersistentModelIndex> indexes);

private slots:
	void endPopulateTree();

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
