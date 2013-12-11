#include "libraryitemdelegate.h"

#include <settings.h>
#include "librarytreeview.h"
#include "playlists/starrating.h"

#include <QtDebug>
#include <QImageReader>

LibraryItemDelegate::LibraryItemDelegate(LibraryFilterProxyModel *proxy) :
	QStyledItemDelegate(proxy), _proxy(proxy)
{

}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	/*const QStandardItemModel *model = qobject_cast<const QStandardItemModel*>(_proxy->sourceModel());
	QStandardItem *item = model->itemFromIndex(_proxy->mapToSource(index));
	QStyleOptionViewItemV4 o = option;
	initStyleOption(&o, index);

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	if (item->type() == LibraryItem::Track && Settings::getInstance()->isStarDelegates()) {
		QStandardItem *track = static_cast<QStandardItem*>(item);
		FileHelper fh(track->absoluteFilePath());
		if (fh.rating() > 0) {
			StarRating starRating(fh.rating());
			starRating.paint(painter, o.rect, o.palette, StarRating::ReadOnly);
		}
	}
	switch (item->type()) {
	case LibraryItem::Album:
		this->drawAlbum(painter, o, item);
		break;
	case LibraryItem::Artist:
		this->drawArtist(painter, o, index);
		break;
	case LibraryItem::Disc:
		this->drawDisc(painter, o, index);
		break;
	case LibraryItem::Letter:
		this->drawLetter(painter, o, index);
		break;
	case LibraryItem::Track:
		this->drawTrack(painter, o, item);
		break;
	default:
		QStyledItemDelegate::paint(painter, o, index);
		break;
	}*/
}

/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	/*Settings *settings = Settings::getInstance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	if (settings->isCoversEnabled() && item->type() == LibraryItem::Album) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}*/
}

void LibraryItemDelegate::drawAlbum(QPainter *painter, const QStyleOptionViewItem &option, QStandardItem *item) const
{
	/*static QImageReader imageReader;
	// Albums have covers usually
	if (item->icon().isNull()) {
		Settings *settings = Settings::getInstance();
		imageReader.setFileName(item->absolutePath() + '/' + item->coverFileName());
		imageReader.setScaledSize(QSize(settings->coverSize(), settings->coverSize()));
		item->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
	}*/
	QStyledItemDelegate::paint(painter, option, item->index());
}

void LibraryItemDelegate::drawArtist(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter, option, index);
}

void LibraryItemDelegate::drawDisc(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const
{
	option.state = QStyle::State_None;
	QPointF p1 = option.rect.bottomLeft(), p2 = option.rect.bottomRight();
	p1.setX(p1.x() + 2);
	p2.setX(p2.x() - 2);
	painter->setPen(Qt::gray);
	painter->drawLine(p1, p2);
	QStyledItemDelegate::paint(painter, option, index);
}

void LibraryItemDelegate::drawLetter(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// One cannot interact with an alphabetical separator
	option.state = QStyle::State_None;
	option.font.setBold(true);
	QPointF p1 = option.rect.bottomLeft(), p2 = option.rect.bottomRight();
	p1.setX(p1.x() + 2);
	p2.setX(p2.x() - 2);
	painter->setPen(Qt::gray);
	painter->drawLine(p1, p2);
	QStyledItemDelegate::paint(painter, option, index);
}

void LibraryItemDelegate::drawTrack(QPainter *painter, QStyleOptionViewItem &option, const QStandardItem *track) const
{
	/// XXX: it will be a piece of cake to add an option that one can customize how track number will be displayed
	/// QString title = settings->libraryItemTitle();
	/// for example: zero padding
	/*QString title = QString("%1").arg(track->trackNumber(), 2, 10, QChar('0')).append(". ").append(track->text());
	option.text = title;
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);*/
}
