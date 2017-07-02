#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <model/trackdao.h>
#include <model/sqldatabase.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include <treeview.h>

#include "libraryitemdelegate.h"
#include "libraryitemmodel.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include "miamlibrary_global.hpp"

/// Forward declarations
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
 * \details     Multiple hierarchies can be chosen by one with a right click on the header: Artist / Album / Track, Year / Artist - Album, etc.
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

	/** Cache of expanded albums and their covers. */
	QMap<AlbumItem*, QImage*> _expandedCovers;

	/** This view uses a proxy to specify how items in the Tree should be ordered together. */
	MiamSortFilterProxyModel *_proxyModel;

	LibraryItemDelegate *_delegate;

	QTranslator translator;

public:
	/** Extendable context menu shown on screen to dispatch tracks (or albums, etc) to Playlist,
	 * Tag Editor, and custom plugin defined actions. */
	QMenu *properties;

	explicit LibraryTreeView(QWidget *parent = nullptr);

	virtual ~LibraryTreeView();

	const QImage *expandedCover(AlbumItem *album) const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QList<QUrl> *tracks) const override;

	inline JumpToWidget* jumpToWidget() const { return _jumpToWidget; }

	inline LibraryItemModel* model() const { return _libraryModel; }

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	/** Redefined to disable search in the table and trigger jumpToWidget's action. */
	virtual void keyboardSearch(const QString &search) override;

	virtual void keyPressEvent(QKeyEvent *event) override;

	virtual void keyReleaseEvent(QKeyEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

private:
	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const override;

	/** Reimplemented. */
	virtual void updateSelectedTracks() override;

public slots:
	/** Invert the current sort order. */
	void changeSortOrder();

	/** Reimplemented. */
	virtual void reset() override;

private slots:
	void endPopulateTree();

	void removeExpandedCover(const QModelIndex &index);

	void scrollToLetter(const QString &letter);

	void setExpandedCover(const QModelIndex &index);

	void updateViewProperty(Settings::ViewProperty vp, const QVariant &);
};

#endif // LIBRARYTREEVIEW_H
