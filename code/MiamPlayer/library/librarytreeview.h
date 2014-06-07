#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
#include <model/librarysqlmodel.h>
#include <settings.h>

#include "../treeview.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>

class LibraryFilterProxyModel;
class CircleProgressBar;
class LibraryItemDelegate;
class JumpToWidget;

/**
 * @brief The LibraryTreeView class is displaying tracks in a tree, where items are sorted in Artists > Albums > Tracks.
 */
class LibraryTreeView : public TreeView
{
	Q_OBJECT

private:
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	QPoint currentPos;
	QMenu *properties;

	QStandardItemModel* _libraryModel;
	LibrarySqlModel *sqlModel;
	LibraryItemDelegate *_itemDelegate;
	QTimer *_timer;

	QHash<QString, QStandardItem*> _artists;
	QHash<QPair<QStandardItem*, QString>, QStandardItem*> _albums;
	QHash<QPair<QStandardItem*, int>, QStandardItem*> _discNumbers;
	QHash<QString, QStandardItem*> _albums2;
	QHash<QString, QStandardItem*> _albumsAbsPath;
	QHash<QString, QStandardItem*> _artistsAlbums;
	QHash<int, QStandardItem*> _years;
	QHash<QString, QStandardItem*> _letters;

	// Letter L returns all Artists (e.g.) starting with L
	QMultiHash<QModelIndex, QModelIndex> _topLevelItems;

	JumpToWidget *_jumpToWidget;

	Q_ENUMS(ItemType)
	Q_ENUMS(DataField)

public:
	explicit LibraryTreeView(QWidget *parent = 0);

	/** For every item in the library, gets the top level letter attached to it. */
	QChar currentLetter() const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	void init(LibrarySqlModel *sql);

	void insertTrackFromFile(const FileHelper &fh);
	void insertTrackFromRecord(const QSqlRecord &record);

	enum ItemType { Artist		= 0,
					Album		= 1,
					ArtistAlbum	= 2,
					Disc		= 3,
					Letter		= 4,
					Track		= 5,
					Year		= 6 };

	// User defined data types (item->setData(QVariant, Field);)
	enum DataField { Type					= Qt::UserRole + 1,
					 DataAbsFilePath		= Qt::UserRole + 2,
					 DataCoverPath			= Qt::UserRole + 3,
					 DataTrackNumber		= Qt::UserRole + 4,
					 DataDiscNumber			= Qt::UserRole + 5,
					 DataNormalizedString	= Qt::UserRole + 6,
					 DataYear				= Qt::UserRole + 7};

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	virtual void drawBranches(QPainter * painter, const QRect & rect, const QModelIndex & index) const;

	virtual void drawRow(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	virtual void paintEvent(QPaintEvent *);

private:
	void bindCoverToAlbum(QStandardItem *itemAlbum, const QString &album, const QString &absFilePath);

	// Thanks StackOverflow for this algorithm (works like a charm without any changes)
	QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false) const;

	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	QStandardItem* insertLetter(const QString &letters);

	void insertTrack(const QString &absFilePath, const QString &artistAlbum, const QString &artist, const QString &album,
					 int discNumber, const QString &title, int trackNumber, int year);

	void updateCover(const QFileInfo &coverFileInfo);

	void repaintIcons();

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
	void reset();

private slots:
	void endPopulateTree();

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
