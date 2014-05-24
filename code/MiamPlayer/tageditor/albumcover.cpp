#include "albumcover.h"

#include <QBuffer>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QUrl>

#include <QtDebug>

AlbumCover::AlbumCover(QWidget *parent) :
	QWidget(parent), _cover(NULL), _subMenuApplyTo(NULL)
{
	this->setAcceptDrops(true);
	_imageMenu = new QMenu(this);

	// Actions displayed in any context
	QAction *loadCoverAction = _imageMenu->addAction(tr("Load a new cover..."));
	_extractCoverAction = _imageMenu->addAction(tr("Extract current cover..."));
	_removeCoverAction = _imageMenu->addAction(tr("Remove cover"));
	connect(loadCoverAction, &QAction::triggered, this, &AlbumCover::loadCover);
	connect(_extractCoverAction, &QAction::triggered, this, &AlbumCover::extractCover);
	connect(_removeCoverAction, &QAction::triggered, this, &AlbumCover::removeCover);
}

/** Displays a cover in the tag editor. */
void AlbumCover::setCover(Cover *cover)
{
	_cover = cover;
	update();
}

/** Puts a default picture in this widget. */
void AlbumCover::resetCover()
{
	_cover = NULL;
	update();
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
		emit coverHasChanged(_cover);
	}
	update();
}

/** Redefined to display a small context menu in the view. */
void AlbumCover::contextMenuEvent(QContextMenuEvent *event)
{
	bool isDefaultCover = (_cover == NULL);
	_removeCoverAction->setDisabled(isDefaultCover);
	_extractCoverAction->setDisabled(isDefaultCover);

	// Adapt the context menu to the content of the table
	if (isCoverForUniqueAlbum) {
		if (_applyCoverToCurrentAlbumAction) {
			delete _applyCoverToCurrentAlbumAction;
		}
		_applyCoverToCurrentAlbumAction = _imageMenu->addAction(tr("Apply cover to '%1'").arg(_album));
		_applyCoverToCurrentAlbumAction->setDisabled(isDefaultCover);

		connect(_applyCoverToCurrentAlbumAction, &QAction::triggered, this, &AlbumCover::applyCoverToAll);
	} else {
		if (_subMenuApplyTo) {
			delete _subMenuApplyTo;
		}
		_subMenuApplyTo = _imageMenu->addMenu(tr("Apply cover"));
		QAction *applyCoverToAlbumOnlyAction = _subMenuApplyTo->addAction(tr("to '%1' only").arg(_album));
		QAction *applyCoverToAllAction = _subMenuApplyTo->addAction(tr("to every tracks"));
		_subMenuApplyTo->setDisabled(isDefaultCover);

		connect(applyCoverToAllAction, &QAction::triggered, this, &AlbumCover::applyCoverToAll);
		connect(applyCoverToAlbumOnlyAction, &QAction::triggered, this, &AlbumCover::applyCoverToAlbumOnly);
	}
	_imageMenu->exec(event->globalPos());
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
void AlbumCover::paintEvent(QPaintEvent *)
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
	emit coverHasChanged(_cover);
	repaint();
}

/** Loads a file from the filesystem. */
void AlbumCover::loadCover()
{
	QString newCover = QFileDialog::getOpenFileName(this, tr("Load a new cover"),
		QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(), tr("Images (*.png *.jpg)"));
	if (!newCover.isEmpty()) {
		this->createPixmapFromFile(newCover);
	}
}

/** Allows one to save the current cover to it's filesystem. */
void AlbumCover::extractCover()
{
	QString imageName = QFileDialog::getSaveFileName(this, tr("Save a cover"),
		QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),	tr("Image (*.jpg)"));
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
