#include "libraryitemdelegate.h"

#include "settings.h"
#include "libraryitem.h"
#include "librarytreeview.h"

#include <QtDebug>
#include <QImageReader>

LibraryItemDelegate::LibraryItemDelegate(LibraryFilterProxyModel *proxy) :
	QStyledItemDelegate(proxy), _proxy(proxy)
{
	_libraryModel = static_cast<LibraryModel*>(_proxy->sourceModel());
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	LibraryItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	Settings *settings = Settings::getInstance();
	QStyleOptionViewItemV4 o = option;
	static QImageReader imageReader;
	if (item->type() == LibraryItem::Track && settings->isStarDelegates()) {
		LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
		FileHelper fh(track->absoluteFilePath());
		if (fh.rating() > 0) {
			StarRating starRating(fh.rating());
			starRating.paint(painter, option.rect, option.palette, StarRating::ReadOnly);
		}
	}
	if (item->type() == LibraryItem::Letter) {

		// One cannot interact with an alphabetical separator
		o.state = QStyle::State_None;
		o.font.setBold(true);
		QPointF p1 = o.rect.bottomLeft(), p2 = o.rect.bottomRight();
		p1.setX(p1.x() + 2);
		p2.setX(p2.x() - 2);
		painter->setPen(Qt::gray);
		painter->drawLine(p1, p2);
	} else if (item->type() == LibraryItem::Album && settings->withCovers()) {

		// Albums have covers usually
		LibraryItemAlbum *album = static_cast<LibraryItemAlbum*>(item);
		if (album->icon().isNull()) {
			imageReader.setFileName(album->absolutePath() + '/' + album->coverFileName());
			imageReader.setScaledSize(QSize(settings->coverSize(), settings->coverSize()));
			album->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
		}
	}
	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	QStyledItemDelegate::paint(painter, o, index);
}


/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Settings *settings = Settings::getInstance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	if (settings->withCovers() && item->type() == LibraryItem::Album) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}
