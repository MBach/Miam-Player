#ifndef LIBRARYTREEVIEW_H
#define LIBRARYTREEVIEW_H

#include <filehelper.h>
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
	LibraryFilterProxyModel *proxyModel;
	CircleProgressBar *circleProgressBar;
	QPoint currentPos;
	QMenu *properties;

	QStandardItemModel* _libraryModel;
	LibrarySqlModel *sqlModel;

	QMap<QString, QStandardItem*> _artists;
	QHash<QPair<QStandardItem*, QString>, QStandardItem*> _albums;
	QHash<QPair<QStandardItem*, int>, QStandardItem*> _discNumbers;
	QHash<QString, QStandardItem*> _albums2;
	QHash<QString, QStandardItem*> _albumsAbsPath;
	QHash<QString, QStandardItem*> _artistsAlbums;
	QHash<int, QStandardItem*> _years;
	QSet<QString> _letters;

	Q_ENUMS(ItemType)
	Q_ENUMS(Field)

public:
	explicit LibraryTreeView(QWidget *parent = 0);

	void init(LibrarySqlModel *sql);

	void insertTrackFromFile(const FileHelper &fh);
	void insertTrackFromRecord(const QSqlRecord &record);

	enum ItemType { Artist = 0,
					Album = 1,
					ArtistAlbum = 2,
					Disc = 3,
					Letter = 4,
					Track = 5,
					Year = 6 };

	// User defined data types (item->setData(QVariant, Field);)
	enum Field { Type = Qt::UserRole + 1,
				 AbsFilePath = Qt::UserRole + 2,
				 CoverPath = Qt::UserRole + 3,
				 TrackNumber = Qt::UserRole + 4};

protected:
	/** Redefined to display a small context menu in the view. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	/** Redefined from the super class to add 2 behaviours depending on where the user clicks. */
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

private:
	void bindCoverToAlbum(QStandardItem *itemAlbum, const QString &album, const QString &absFilePath);

	/** Recursive count for leaves only. */
	int count(const QModelIndex &index) const;

	/** Reimplemented. */
	virtual int countAll(const QModelIndexList &indexes) const;

	/** Reimplemented. */
	virtual void findAll(const QPersistentModelIndex &index, QStringList &tracks);

	void insertLetter(const QString &letters);

	void insertTrack(const QString &absFilePath, const QString &artistAlbum, const QString &artist, const QString &album,
					 int discNumber, const QString &title, int trackNumber, int year);

	void updateCover(const QFileInfo &coverFileInfo);

public slots:
	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

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
