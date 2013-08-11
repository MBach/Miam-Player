#include "libraryitemdelegate.h"

#include "settings.h"
#include "libraryitem.h"
#include "librarytreeview.h"
#include "playlists/starrating.h"

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
	QStyleOptionViewItemV4 o = option;
	initStyleOption(&o, index);

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	if (item->type() == LibraryItem::Track && Settings::getInstance()->isStarDelegates()) {
		LibraryItemTrack *track = static_cast<LibraryItemTrack*>(item);
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
	case LibraryItem::Letter:
		this->drawLetter(painter, o, index);
		break;
	case LibraryItem::Track:
		this->drawTrack(painter, o, item);
		break;
	}
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

void LibraryItemDelegate::drawAlbum(QPainter *painter, const QStyleOptionViewItem &option, LibraryItem *item) const
{
	static QImageReader imageReader;
	// Albums have covers usually
	LibraryItemAlbum *album = static_cast<LibraryItemAlbum*>(item);
	if (album->icon().isNull()) {
		Settings *settings = Settings::getInstance();
		imageReader.setFileName(album->absolutePath() + '/' + album->coverFileName());
		imageReader.setScaledSize(QSize(settings->coverSize(), settings->coverSize()));
		album->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
	}
	QStyledItemDelegate::paint(painter, option, item->index());
}

void LibraryItemDelegate::drawArtist(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
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

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

void LibraryItemDelegate::drawTrack(QPainter *painter, QStyleOptionViewItem &option, const LibraryItem *item) const
{
	const LibraryItemTrack *track = static_cast<const LibraryItemTrack*>(item);
	/// XXX: it will be a piece of cake to add an option that one can customize how track number will be displayed
	/// QString title = settings->libraryItemTitle();
	/// for example: zero padding
	QString title = QString("%1").arg(track->trackNumber(), 2, 10, QChar('0')).append(". ").append(track->text());
	option.text = title;
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

	/*painter->save();
	QTextDocument doc;
	doc.setHtml(title);

	option.text = "";
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

	// shift text right to make icon visible
	QSize iconSize = option.icon.actualSize(option.rect.size());
	painter->translate(option.rect.left() + iconSize.width(), option.rect.top());
	QRect clip(0, 0, option.rect.width() + iconSize.width(), option.rect.height());

	painter->setClipRect(clip);
	QAbstractTextDocumentLayout::PaintContext ctx;
	// set text color to red for selected item
	ctx.palette.setColor(QPalette::Text, QColor("red"));
	ctx.clip = clip;
	doc.documentLayout()->draw(painter, ctx);
	painter->restore();*/
}
