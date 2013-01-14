#include "albumcover.h"

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

	// Context menu
	imageMenu = new QMenu(this);
	QAction *resetCoverAction = imageMenu->addAction(tr("Remove cover"));
	QAction *loadCoverAction = imageMenu->addAction(tr("Load a new cover"));
	connect(resetCoverAction, SIGNAL(triggered()), this, SLOT(resetCover()));
	connect(loadCoverAction, SIGNAL(triggered()), this, SLOT(loadCover()));
}

void AlbumCover::createPixmapFromFile(const QString &fileName)
{
	bool isImage = false;
	QImageReader imageReader(fileName);
	imageReader.setDecideFormatFromContent(true);
	if (imageReader.canRead()) {
		isImage = true;
		QPixmap pixmap(fileName);
		if (!pixmap.isNull()) {
			this->setPixmap(pixmap);
			emit coverHasChanged();
		}
	}
}

/** Redefined to display a small context menu in the view. */
void AlbumCover::contextMenuEvent(QContextMenuEvent *event)
{
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

void AlbumCover::resetCover()
{
	this->setPixmap(QPixmap(":/icons/disc"));
	emit coverHasChanged();
}

void AlbumCover::loadCover()
{
	qDebug() << "todo loadCover";
	QString newCover = QFileDialog::getOpenFileName(this, tr("Load a new cover"));
	if (!newCover.isEmpty()) {
		this->createPixmapFromFile(newCover);
	}
}
