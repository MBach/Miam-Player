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
	QWidget(parent)
{
	setAcceptDrops(true);
	imageMenu = new QMenu(this);
}

/** Displays a cover in the tag editor. */
void AlbumCover::setCover(const Cover &cover)
{
	_cover = cover;
	repaint();
}

/** Puts a default picture in this widget. */
void AlbumCover::resetCover()
{
	//_cover.load(":/icons/disc");
	repaint();
}

/** Creates a picture after one has chosen a picture on it's filesystem. */
void AlbumCover::createPixmapFromFile(const QString &fileName)
{
	/*if (_cover.load(fileName)) {
		emit coverHasChanged();
	}*/
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

	/// FIXME
	//bool isCustomPixmap = (_cover.byteArray() != defaultCover.byteArray());
	bool isCustomPixmap = true;
	removeCoverAction->setEnabled(isCustomPixmap);
	extractCoverAction->setEnabled(isCustomPixmap);

	// Adapt the context menu to the content of the table
	if (isCoverForUniqueAlbum) {
		QAction *applyCoverToCurrentAlbumAction = imageMenu->addAction(tr("Apply cover to '%1'").arg(_cover.album()));
		applyCoverToCurrentAlbumAction->setEnabled(isCustomPixmap);

		connect(applyCoverToCurrentAlbumAction, SIGNAL(triggered()), this, SLOT(applyCoverToAll()));
	} else {
		QMenu *subMenuApplyTo = imageMenu->addMenu(tr("Apply cover"));
		QAction *applyCoverToAlbumOnlyAction = subMenuApplyTo->addAction(tr("to '%1' only").arg(_cover.album()));
		QAction *applyCoverToAllAction = subMenuApplyTo->addAction(tr("to every tracks"));
		subMenuApplyTo->setEnabled(isCustomPixmap);

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
	if (_cover.byteArray().isNull()) {
		painter.drawPixmap(rect(), QPixmap(":/icons/disc"));
	} else {
		QPixmap p;
		p.loadFromData(_cover.byteArray(), _cover.format());
		painter.drawPixmap(rect(), p);
	}
}

/** Removes the current cover from this object, and in the table. */
void AlbumCover::removeCover()
{
	this->resetCover();
	emit aboutToRemoveCoverFromTag();
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
		/*if (image.open(QIODevice::WriteOnly) && _cover.save(&image)) {
			image.close();
		}*/
	}
}

/** Apply the current cover to every tracks in the tag editor. */
void AlbumCover::applyCoverToAll()
{
	emit aboutToApplyCoverToAll(true);
}

/** Apply the current cover to related tracks. */
void AlbumCover::applyCoverToAlbumOnly()
{
	emit aboutToApplyCoverToAll(false);
}
