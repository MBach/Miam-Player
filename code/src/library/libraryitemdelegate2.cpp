#include "libraryitemdelegate2.h"

#include "settings.h"

#include <QImageReader>

#include <QtDebug>

LibraryItemDelegate2::LibraryItemDelegate2(LibraryFilterProxyModel *proxy) :
	QItemDelegate(proxy), _proxy(proxy)
{
	_libraryModel = static_cast<LibraryModel*>(_proxy->sourceModel());
}

/** XXX: Split this method for each type of item. */
void LibraryItemDelegate2::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	LibraryItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	QStyleOptionViewItemV4 o = option;
	Settings *settings = Settings::getInstance();
	o.font = settings->font(Settings::LIBRARY);
	static QImageReader imageReader;
	if (item->type() == LibraryItem::Track && settings->isStarDelegates()) {
		LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
		FileHelper fh(track->absoluteFilePath());
		if (fh.rating() > 0) {
			StarRating starRating(fh.rating());
			starRating.paint(painter, option.rect, option.palette, StarRating::ReadOnly);
		}
	}
	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	painter->save();
	if (item->type() == LibraryItem::Letter) {
		// One cannot interact with an alphabetical separator
		o.state = QStyle::State_None;

		this->drawDisplay(painter, o, o.rect, index.data().toString());

		o.font.setBold(true);
		QPointF p1 = o.rect.bottomLeft(), p2 = o.rect.bottomRight();
		p1.setX(p1.x() + 2);
		p2.setX(p2.x() - 2);
		painter->setPen(Qt::gray);
		painter->drawLine(p1, p2);
	} else if (item->type() == LibraryItem::Album && settings->withCovers()) {

		// Albums have covers usually
		LibraryItemAlbum *album = static_cast<LibraryItemAlbum*>(item);
		_libraryModel->indexFromItem(album);
		QPixmap pixmap;
		if (album->icon().isNull()) {
			imageReader.setFileName(album->absolutePath() + '/' + album->coverFileName());
			imageReader.setScaledSize(QSize(settings->coverSize(), settings->coverSize()));
			pixmap = QPixmap::fromImage(imageReader.read());
			album->setIcon(QIcon(pixmap));
			_proxy->setData(index, pixmap, Qt::DecorationRole);
		}
		this->drawFocus(painter, o, o.rect);
		if (pixmap.isNull()) {
			this->drawDisplay(painter, o, o.rect, index.data().toString());
		} else {
			QRect iconRect(o.rect.left() + 1, o.rect.top() + 1, settings->coverSize(), settings->coverSize());
			QRect albumTextRect(iconRect.right(), o.rect.top(), o.rect.width() - settings->coverSize(), o.rect.height());
			this->drawDisplay(painter, o, albumTextRect, index.data().toString());
			this->drawDecoration(painter, o, iconRect, pixmap);
		}
	} else if (item->type() == LibraryItem::Track) {
		this->drawFocus(painter, o, o.rect);
		LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
		/// XXX: it will be a piece of cake to add an option that one can customize how track number will be displayed
		/// QString title = settings->libraryItemTitle();
		/// for example: zero padding
		QString title = QString("%1").arg(track->trackNumber(), 2, 10, QChar('0')).append(". ").append(track->text());
		//QString title = QString::number(track->trackNumber()).append(". ").append(track->text());
		this->drawDisplay(painter, o, o.rect, title);
	} else if (item->type() == LibraryItem::Artist) {
		this->drawFocus(painter, o, o.rect);
		this->drawDisplay(painter, o, o.rect, index.data().toString());
	}
	painter->restore();
}


/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate2::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Settings *settings = Settings::getInstance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	if (settings->withCovers() && item->type() == LibraryItem::Album) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QItemDelegate::sizeHint(option, index);
	}
}
