#include "albumcover.h"

#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QImageReader>
#include <QPixmap>
#include <QUrl>

#include <QtDebug>

AlbumCover::AlbumCover(QWidget *parent) :
    QLabel(parent)
{
	setAcceptDrops(true);

	// Context menu...
	imageMenu = new QMenu(this);
	loadCoverAction = imageMenu->addAction(tr("Load a new cover..."));
	extractCoverAction = imageMenu->addAction(tr("Extract current cover..."));
	removeCoverAction = imageMenu->addAction(tr("Remove cover"));

	// ...with a submenu
	subMenuApplyTo = imageMenu->addMenu(tr("Apply cover"));
	applyCoverToAllAction = subMenuApplyTo->addAction(tr("to every tracks"));
	applyCoverToAlbumOnlyAction = subMenuApplyTo->addAction(tr("to this album only"));

	connect(loadCoverAction, SIGNAL(triggered()), this, SLOT(loadCover()));
	connect(extractCoverAction, SIGNAL(triggered()), this, SLOT(extractCover()));
	connect(removeCoverAction, SIGNAL(triggered()), this, SLOT(removeCover()));
	connect(applyCoverToAllAction, SIGNAL(triggered()), this, SLOT(applyCoverToAll()));
	connect(applyCoverToAlbumOnlyAction, SIGNAL(triggered()), this, SLOT(applyCoverToAlbumOnly()));

	defaultPixmap = QPixmap(QString::fromUtf8(":/icons/disc"));
}

/** Displays a cover in the tag editor. */
void AlbumCover::displayFromAttachedPicture(const QVariant &cover)
{
	QPixmap p;
	if (p.loadFromData(cover.toByteArray())) {
		this->setPixmap(p);
	} else {
		this->setPixmap(defaultPixmap);
	}
}

/** Creates a picture after one has chosen a picture on it's filesystem. */
void AlbumCover::createPixmapFromFile(const QString &fileName)
{
	QImageReader imageReader(fileName);
	imageReader.setDecideFormatFromContent(true);
	if (imageReader.canRead()) {
		QPixmap pixmap(fileName);
		if (!pixmap.isNull()) {
			this->setPixmap(pixmap);
		}
	}
}

/** Redefined to display a small context menu in the view. */
void AlbumCover::contextMenuEvent(QContextMenuEvent *event)
{
	bool pixmapsAreEquals = (pixmap()->toImage() == defaultPixmap.toImage());
	removeCoverAction->setDisabled(pixmapsAreEquals);
	extractCoverAction->setDisabled(pixmapsAreEquals);
	subMenuApplyTo->setDisabled(pixmapsAreEquals);
	imageMenu->exec(event->globalPos());
}

void AlbumCover::dragEnterEvent(QDragEnterEvent *event)
{
	// If the source of the drag and drop is another application
	if (event->source() == NULL) {
		event->acceptProposedAction();
	}
}

void AlbumCover::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void AlbumCover::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.size() == 1 && urls.first().isLocalFile()) {
		this->createPixmapFromFile(urls.first().toLocalFile());
	}
}

/** Removes the current cover (puts a default picture instead). */
void AlbumCover::removeCover()
{
	this->setPixmap(defaultPixmap);
	// Do not send a signal when it was not explicitly asked by one, like deselecting a track,
	// or loading new tracks in the tag editor
	if (sender() == removeCoverAction) {
		emit aboutToRemoveCoverFromTag();
	}
}

/** Loads a file from the filesystem. */
void AlbumCover::loadCover()
{
	QString newCover = QFileDialog::getOpenFileName(this, tr("Load a new cover"),
		QDesktopServices::storageLocation(QDesktopServices::MusicLocation), tr("Images (*.png, *.jpg)"));
	if (!newCover.isEmpty()) {
		this->createPixmapFromFile(newCover);
	}
}

/** Allows one to save the current cover to it's filesystem. */
void AlbumCover::extractCover()
{
	QString imageName = QFileDialog::getSaveFileName(this, tr("Save a cover"),
		QDesktopServices::storageLocation(QDesktopServices::MusicLocation),	tr("Image (*.jpg)"));
	if (!imageName.isEmpty()) {
		QFile image(imageName);
		if (image.open(QIODevice::WriteOnly) && pixmap()->save(&image)) {
			image.close();
		}
	}
}

/** Apply the current cover to every tracks in the tag editor. */
void AlbumCover::applyCoverToAll()
{
	qDebug() << "AlbumCover::applyCoverToAll() not yet implemented";
}

/** Apply the current cover to related tracks. */
void AlbumCover::applyCoverToAlbumOnly()
{
	qDebug() << "AlbumCover::applyCoverToAlbumOnly() not yet implemented";
}
