#ifndef ALBUMCOVER_H
#define ALBUMCOVER_H

#include <QMenu>

/**
 * @brief The AlbumCover class is used to manipulate cover albums inside music files.
 */
class AlbumCover : public QWidget
{
    Q_OBJECT
private:
	QMenu *imageMenu;

	QPixmap defaultPixmap;
	QPixmap pixmap;

	bool isCoverForUniqueAlbum;
	QString album;

public:
	AlbumCover(QWidget *parent = 0);

	/** Displays a cover in the tag editor. */
	void setImageData(const QVariant &cover, const QString &albumName);

	void setCoverForUniqueAlbum(bool isUnique) { isCoverForUniqueAlbum = isUnique; }

	/** Puts a default picture in this widget. */
	void resetCover();

	QVariant coverData() const;

private:
	/** Creates a picture after one has chosen a picture on it's filesystem. */
	void createPixmapFromFile(const QString &fileName);

protected:
	/** Redefined to display a small context menu in the view. */
	void contextMenuEvent(QContextMenuEvent *event);

	/** Redefined. */
	void dragEnterEvent(QDragEnterEvent *event);

	/** Redefined. */
	void dragMoveEvent(QDragMoveEvent *event);

	/** Allows one to drag & drop pictures from external software. */
	void dropEvent(QDropEvent *event);

	/** Redefined to switch between images very quickly. */
	void paintEvent(QPaintEvent *);

private slots:

	/** Loads a file from the filesystem. */
	void loadCover();

	/** Removes the current cover from this object, and in the table. */
	void removeCover();

	/** Allows one to save the current cover to it's filesystem. */
	void extractCover();

	/** Apply the current cover to every tracks in the tag editor. */
	void applyCoverToAll();

	/** Apply the current cover to related tracks. */
	void applyCoverToAlbumOnly();

signals:
	/** This signal is sent to the TagEditorTableWidget class to apply the selected cover to the album only or to everything. */
	void aboutToApplyCoverToAll(bool);

	/** This signal is sent to the TagEditorTableWidget class to remove the cover. */
	void aboutToRemoveCoverFromTag();

	void coverHasChanged();
};

#endif // ALBUMCOVER_H
