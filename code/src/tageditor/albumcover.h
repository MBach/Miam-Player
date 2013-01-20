#ifndef ALBUMCOVER_H
#define ALBUMCOVER_H

#include <QLabel>
#include <QMenu>

/**
 * @brief The AlbumCover class is used to manipulate cover albums inside music files.
 */
class AlbumCover : public QLabel
{
    Q_OBJECT
private:
	QMenu *imageMenu;
	QAction *loadCoverAction;
	QAction *extractCoverAction;
	QAction *removeCoverAction;

	QMenu *subMenuApplyTo;
	QAction *applyCoverToAllAction;
	QAction *applyCoverToAlbumOnlyAction;

	QPixmap defaultPixmap;

public:
	AlbumCover(QWidget *parent = 0);

	/** Displays a cover in the tag editor. */
	void displayFromAttachedPicture(const QVariant &cover);

private:
	/** Creates a picture after one has chosen a picture on it's filesystem. */
	void createPixmapFromFile(const QString &fileName);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

public slots:
	/** Removes the current cover (puts a default picture instead). */
	void removeCover();

private slots:
	/** Loads a file from the filesystem. */
	void loadCover();

	/** Allows one to save the current cover to it's filesystem. */
	void extractCover();

	/** Apply the current cover to every tracks in the tag editor. */
	void applyCoverToAll();

	/** Apply the current cover to related tracks. */
	void applyCoverToAlbumOnly();

signals:
	void aboutToRemoveCoverFromTag();
};

#endif // ALBUMCOVER_H
