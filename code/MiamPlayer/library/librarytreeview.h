#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
#include <model/trackdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>

#include "../treeview.h"
#include "libraryitemmodel.h"

#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>

class CircleProgressBar;
class JumpToWidget;
class LibraryFilterLineEdit;
class LibraryFilterProxyModel;
class LibraryItemDelegate;
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
	LibraryItemModel *_libraryModel;

	/** This class has its own delegate because each level of the tree has a very specific way to render itself on screen. */
	LibraryItemDelegate *_itemDelegate;

	/** Shortcut widget to navigate quickly in a big treeview. */
	JumpToWidget *_jumpToWidget;

	/** Cache of expanded albums and their covers. */
	QMap<QStandardItem*, QImage*> _expandedCovers;

	LibraryFilterLineEdit *_searchBar;

public:
	QShortcut *sendToCurrentPlaylist;
	QShortcut *openTagEditor;

	explicit LibraryTreeView(QWidget *parent = 0);

	/** For every item in the library, gets the top level letter attached to it. */
	QChar currentLetter() const;

	const QImage * expandedCover(QStandardItem *album) const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	void findMusic(const QString &text);

	virtual void init() override;

	inline JumpToWidget* jumpToWidget() const { return _jumpToWidget; }

	inline LibraryItemModel* model() const { return _libraryModel; }

	void setSearchBar(LibraryFilterLineEdit *lfle);

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

	/** Reload covers when one has changed cover size in options. */
	void reloadCovers();

	/** Reimplemented. */
	virtual void reset() override;

private slots:
	void endPopulateTree();

	void removeExpandedCover(const QModelIndex &index);

	void setExpandedCover(const QModelIndex &index);

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);
};

#endif // LIBRARYTREEVIEW_H
