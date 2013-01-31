#include "albumcover.h"

#include <QBuffer>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QUrl>

#include <QtDebug>

AlbumCover::AlbumCover(QWidget *parent) :
	QWidget(parent), _cover(NULL)
{
	setAcceptDrops(true);
	imageMenu = new QMenu(this);
}

/** Displays a cover in the tag editor. */
void AlbumCover::setCover(Cover *cover)
{
	_cover = cover;
	repaint();
}

/** Puts a default picture in this widget. */
void AlbumCover::resetCover()
{
	_cover = NULL;
	repaint();
}

/** Creates a picture after one has chosen a picture on it's filesystem. */
void AlbumCover::createPixmapFromFile(const QString &fileName)
{
	/// XXX
	if (_cover != NULL) {
		delete _cover;
	}
	_cover = new Cover(fileName);
	if (!_cover->byteArray().isEmpty()) {
		emit coverHasChanged();
	}
	repaint();
}

/** Redefined to display a small context menu in the view. */
void AlbumCover::contextMenuEvent(QContextMenuEvent *event)
{
	if (!imageMenu->isEmpty()) {
		imageMenu->clear();
	}

	// Actions displayed in any context
	QAction *loadCoverAction = imageMenu->addAction(tr("Load a new cover..."));
	QAction *extractCoverAction = imageMenu->addAction(tr("Extract current cover..."));
	QAction *removeCoverAction = imageMenu->addAction(tr("Remove cover"));
	connect(loadCoverAction, SIGNAL(triggered()), this, SLOT(loadCover()));
	connect(extractCoverAction, SIGNAL(triggered()), this, SLOT(extractCover()));
	connect(removeCoverAction, SIGNAL(triggered()), this, SLOT(removeCover()));

	bool isDefaultCover = (_cover == NULL);
	removeCoverAction->setDisabled(isDefaultCover);
	extractCoverAction->setDisabled(isDefaultCover);

	// Adapt the context menu to the content of the table
	if (isCoverForUniqueAlbum) {
		QAction *applyCoverToCurrentAlbumAction = imageMenu->addAction(tr("Apply cover to '%1'").arg(_album));
		applyCoverToCurrentAlbumAction->setDisabled(isDefaultCover);

		connect(applyCoverToCurrentAlbumAction, SIGNAL(triggered()), this, SLOT(applyCoverToAll()));
	} else {
		QMenu *subMenuApplyTo = imageMenu->addMenu(tr("Apply cover"));
		QAction *applyCoverToAlbumOnlyAction = subMenuApplyTo->addAction(tr("to '%1' only").arg(_album));
		QAction *applyCoverToAllAction = subMenuApplyTo->addAction(tr("to every tracks"));
		subMenuApplyTo->setDisabled(isDefaultCover);

		connect(applyCoverToAllAction, SIGNAL(triggered()), this, SLOT(applyCoverToAll()));
		connect(applyCoverToAlbumOnlyAction, SIGNAL(triggered()), this, SLOT(applyCoverToAlbumOnly()));
	}
	imageMenu->exec(event->globalPos());
}

/** Redefined. */
void AlbumCover::dragEnterEvent(QDragEnterEvent *event)
{
	// If the source of the drag and drop is another application
	if (event->source() == NULL) {
		event->acceptProposedAction();
	}
}

/** Redefined. */
void AlbumCover::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

/** Allows one to drag & drop pictures from external software. */
void AlbumCover::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.size() == 1 && urls.first().isLocalFile()) {
		this->createPixmapFromFile(urls.first().toLocalFile());
	}
}

/** Redefined to switch between images very quickly. */
void AlbumCover::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	if (_cover == NULL || _cover->byteArray().isEmpty()) {
		painter.drawPixmap(rect(), QPixmap(":/icons/disc"));
	} else {
		QPixmap p;
		if (p.loadFromData(_cover->byteArray(), _cover->format())) {
			painter.drawPixmap(rect(), p);
		} else {
			// Couldn't load the cover, using default one
			painter.drawPixmap(rect(), QPixmap(":/icons/disc"));
		}
	}
}

/** Removes the current cover from this object, and in the table. */
void AlbumCover::removeCover()
{
	_cover = NULL;
	emit aboutToRemoveCoverFromTag();
	repaint();
}

/** Loads a file from the filesystem. */
void AlbumCover::loadCover()
{
	QString newCover = QFileDialog::getOpenFileName(this, tr("Load a new cover"),
		QDesktopServices::storageLocation(QDesktopServices::MusicLocation), tr("Images (*.png *.jpg)"));
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
		if (image.open(QIODevice::WriteOnly)) {
			image.write(_cover->byteArray());
			image.close();
		}
	}
}

/** Apply the current cover to every tracks in the tag editor. */
void AlbumCover::applyCoverToAll()
{
	emit aboutToApplyCoverToAll(true, _cover);
}

/** Apply the current cover to related tracks. */
void AlbumCover::applyCoverToAlbumOnly()
{
	emit aboutToApplyCoverToAll(false, _cover);
}
