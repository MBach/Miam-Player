#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <QMenu>
#include <QSortFilterProxyModel>

#include <model/librarymodel.h>
#include "circleprogressbar.h"
#include "musicsearchengine.h"
#include "libraryfilterproxymodel.h"
#include "treeview.h"

#include "library/libraryorderdialog.h"

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
	LibraryModel* _libraryModel;

	Settings::InsertPolicy _currentInsertPolicy;

	//test
	QMap<QString, LibraryItemArtist*> _artists;
	QHash<QPair<LibraryItemArtist*, QString>, LibraryItemAlbum*> _albums;
	QHash<QPair<LibraryItemAlbum*, int>, LibraryItemDiscNumber*> _discNumbers;
	QHash<QString, LibraryItemAlbum*> _albums2;
	QHash<QString, LibraryItemAlbum*> _albumsAbsPath;
	QHash<QString, LibraryItemArtist*> _artistsAlbums;
	QHash<int, LibraryItem*> _years;
	QSet<QString> _letters;

public:
	explicit LibraryTreeView(QWidget *parent = 0);

	void init();

	void insertLetter(const QString &letters);

	/** XXX: 8 args, seriously? */
	void insertTrack(const QString &absFilePath, const QString &artist, const QString &artistAlbum, const QString &album,
					 const QString &title, int trackNumber, int discNumber, int year);

	void insertTrack2(PersistentItem *item);

	/** Redefined. */
	virtual void setModel(QAbstractItemModel *model);

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
