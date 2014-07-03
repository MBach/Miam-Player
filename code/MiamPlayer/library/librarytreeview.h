#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
#include <model/librarysqlmodel.h>
#include <settings.h>

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
	QShortcut *sendToCurrentPlaylist;
	QShortcut *openTagEditor;

	explicit LibraryTreeView(QWidget *parent = 0);

	/** For every item in the library, gets the top level letter attached to it. */
	QChar currentLetter() const;

	/** Reimplemented. */
	virtual void findAll(const QModelIndex &index, QStringList &tracks) const;

	virtual void init(LibrarySqlModel *sql);

	void insertTrackFromFile(const FileHelper &fh);
	void insertTrackFromRecord(const QSqlRecord &record);

	enum ItemType { IT_Artist		= 0,
					IT_Album		= 1,
					IT_ArtistAlbum	= 2,
					IT_Disc			= 3,
					IT_Letter		= 4,
					IT_Track		= 5,
					IT_Year			= 6 };

	// User defined data types (item->setData(QVariant, Field);)
	enum DataField { DF_ItemType			= Qt::UserRole + 1,
					 DF_AbsFilePath			= Qt::UserRole + 2,
					 DF_CoverPath			= Qt::UserRole + 3,
					 DF_TrackNumber			= Qt::UserRole + 4,
					 DF_DiscNumber			= Qt::UserRole + 5,
					 DF_NormalizedString	= Qt::UserRole + 6,
					 DF_Year				= Qt::UserRole + 7,
				   /// TEST QSortFilterProxyModel
					 DF_Highlighted			= Qt::UserRole + 8};

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

signals:
	/** (Dis|En)able covers.*/
	void displayCovers(bool);

	void searchMusic();
};

#endif // LIBRARYTREEVIEW_H
