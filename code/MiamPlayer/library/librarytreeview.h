#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
#include <model/trackdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>

#include "../treeview.h"

#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>

class LibraryFilterProxyModel;
class CircleProgressBar;
class LibraryItemDelegate;
class JumpToWidget;
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
class LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	/** This view uses a proxy to specify how items in the Tree should be ordered together. */
	LibraryFilterProxyModel *_proxyModel;

	/**
	 * DEPRECATED: should be replaced with an horizontal progressbar in Modern UI style.
	 * However, future class and this class are not platform independant.
	 */
	CircleProgressBar *_circleProgressBar;

	/** Extendable context menu shown on screen to dispatch tracks (or albums, etc) to Playlist,
	 * Tag Editor, and custom plugin defined actions. */
	QMenu *_properties;

	/** The model used by this treeView is a simple QStandardItemModel. Hard work has been delegated to Proxy and ItemDelegate for the rending. */
	QStandardItemModel* _libraryModel;

	/** This class has its own delegate because each level of the tree has a very specific way to render itself on screen. */
	LibraryItemDelegate *_itemDelegate;

	/** This timer is used to animate album cover when one is scrolling.
	 * It improves reactivity of the UI by temporarily disabling painting events.
	 * When covers are becoming visible once again, they are redisplayed with a nice fading effect. */
	QTimer *_timer;

	/** This map is a kind of cache, used to insert nodes in this tree at the right location. */
	QMap<GenericDAO*, QStandardItem*> _map;

	/** Letters are items to groups separate of top levels items (items without parent). */
	QHash<QString, SeparatorItem*> _letters;

	/** Letter L returns all Artists (e.g.) starting with L. */
	QMultiHash<SeparatorItem*, QModelIndex> _topLevelItems;

	/** Shortcut widget to navigate quickly in a big treeview. */
	JumpToWidget *_jumpToWidget;

	/** Cache of expanded albums and their covers. */
	QMap<QStandardItem*, QImage*> _expandedCovers;

public:
	QShortcut *sendToCurrentPlaylist;
	QShortcut *openTagEditor;

	explicit LibraryTreeView(QWidget *parent = 0);

	/** For every item in the library, gets the top level letter attached to it. */
	QChar currentLetter() const;

	const QImage * expandedCover(QStandardItem *album) const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	virtual void init();

	/** Rebuild the list of separators when one has changed grammatical articles in options. */
	void rebuildSeparators();

	void setVisible(bool visible);

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	SeparatorItem *insertSeparator(const QStandardItem *node);

	void repaintIcons();

	/** Reimplemented. */
	virtual void updateSelectedTracks();

public slots:
	/** Invert the current sort order. */
	void changeSortOrder();

	/** Redraw the treeview with a new display mode. */
	void changeHierarchyOrder();

	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

	/** Find index from current letter then scrolls to it. */
	void jumpTo(const QString &letter);

	/** Reimplemented. */
	virtual void reset();

private slots:
	void endPopulateTree();

	/** Find and insert a node in the hierarchy of items. */
	void insertNode(GenericDAO *node);

	void removeExpandedCover(const QModelIndex &index);

	void setExpandedCover(const QModelIndex &index);

	void updateNode(GenericDAO *node);

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
