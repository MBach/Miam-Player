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

	/// XXX: I think displaying stars directly in the library should be done in a future release.
	/// This idea is a quite good one, but it raises lots of new questions.
	// QPoint currentPos;

	/** Extendable context menu shown on screen to dispatch tracks (or albums, etc) to Playlist,
	 * Tag Editor, and custom plugin defined actions. */
	QMenu *_properties;

	/** The model used by this treeView is a simple QStandardItemModel. Hard work has been delegated to Proxy and ItemDelegate for the rending. */
	QStandardItemModel* _libraryModel;

	/** Link to the database to be able to integrate new tracks in the tree. */
	SqlDatabase *_db;

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
	QMultiHash<QModelIndex, QModelIndex> _topLevelItems;

	/** Shortcut widget to navigate quickly in a big treeview. */
	JumpToWidget *_jumpToWidget;

	Q_ENUMS(ItemType)
	Q_ENUMS(DataField)

public:
	QShortcut *sendToCurrentPlaylist;
	QShortcut *openTagEditor;

	explicit LibraryTreeView(QWidget *parent = 0);

	/** For every item in the library, gets the top level letter attached to it. */
	QChar currentLetter() const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	virtual void init(SqlDatabase *db);

	enum ItemType { IT_Artist		= QMetaType::User + 1,
					IT_Album		= QMetaType::User + 2,
					IT_ArtistAlbum	= QMetaType::User + 3,
					IT_Disc			= QMetaType::User + 4,
					IT_Separator	= QMetaType::User + 5,
					IT_Track		= QMetaType::User + 6,
					IT_Year			= QMetaType::User + 7 };

	// User defined data types (item->setData(QVariant, Field);)
	enum DataField { DF_URI					= Qt::UserRole + 1,
					 DF_CoverPath			= Qt::UserRole + 2,
					 DF_TrackNumber			= Qt::UserRole + 3,
					 DF_DiscNumber			= Qt::UserRole + 4,
					 DF_NormalizedString	= Qt::UserRole + 5,
					 DF_Year				= Qt::UserRole + 6,
				   /// TEST QSortFilterProxyModel
					 DF_Highlighted			= Qt::UserRole + 7,
					 DF_IsRemote			= Qt::UserRole + 8,
					 DF_IconPath			= Qt::UserRole + 9};

	void setVisible(bool visible);

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	virtual void drawBranches(QPainter * painter, const QRect & rect, const QModelIndex & index) const;

	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	///TODO in a future release
	///virtual void mouseDoubleClickEvent(QMouseEvent *event);

	virtual void paintEvent(QPaintEvent *);

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	SeparatorItem *insertSeparator(const QString &letters);

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

	void updateNode(GenericDAO *node);

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
