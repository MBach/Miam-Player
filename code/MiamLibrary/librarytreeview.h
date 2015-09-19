#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
#include <model/trackdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>
#include <treeview.h>

#include "libraryitemmodel.h"

#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include "miamlibrary_global.h"

class CircleProgressBar;
class JumpToWidget;
class LibraryFilterLineEdit;
class LibraryFilterProxyModel;
class ArtistItem;
class AlbumItem;
class DiscItem;
class SeparatorItem;
class TrackItem;
class YearItem;

/**
 * \brief		The LibraryTreeView class is displaying tracks in a tree.
 * \details     Multiple hierarchies can be chosen by one with a right click on the header: Artist \ Album \ Track, Year \ Artist - Album, etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	/** The model used by this treeView is a simple QStandardItemModel. Hard work has been delegated to Proxy and ItemDelegate for the rending. */
	LibraryItemModel *_libraryModel;

	/** Shortcut widget to navigate quickly in a big treeview. */
	JumpToWidget *_jumpToWidget;

	/**
	 * DEPRECATED: should be replaced with an horizontal progressbar in Modern UI style.
	 * However, future class and this class are not platform independant.
	 */
	CircleProgressBar *_circleProgressBar;

	/** Extendable context menu shown on screen to dispatch tracks (or albums, etc) to Playlist,
	 * Tag Editor, and custom plugin defined actions. */
	QMenu *_properties;

	/** Cache of expanded albums and their covers. */
	QMap<AlbumItem*, QImage*> _expandedCovers;

	/** This view uses a proxy to specify how items in the Tree should be ordered together. */
	QSortFilterProxyModel *_proxyModel;

public:
	QShortcut *sendToCurrentPlaylist;
	QShortcut *openTagEditor;

	explicit LibraryTreeView(QWidget *parent = 0);

	void createConnectionsToDB();

	const QImage *expandedCover(AlbumItem *album) const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	void findMusic(const QString &text);

	inline JumpToWidget* jumpToWidget() const { return _jumpToWidget; }

	inline LibraryItemModel* model() const { return _libraryModel; }

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Highlight items in the Tree when one has activated this option in settings. */
	void highlightMatchingText(const QString &text);

	/** Reimplemented. */
	virtual void updateSelectedTracks();

public slots:
	/** Invert the current sort order. */
	void changeSortOrder();

	/** Redraw the treeview with a new display mode. */
	void changeHierarchyOrder();

	/** Find index from current letter then scrolls to it. */
	void jumpTo(const QString &letter);

	/** Reimplemented. */
	virtual void reset() override;

private slots:
	void endPopulateTree();

	void removeExpandedCover(const QModelIndex &index);

	void setExpandedCover(const QModelIndex &index);

signals:
	void aboutToUpdateCoverSize();
};

#endif // LIBRARYTREEVIEW_H
