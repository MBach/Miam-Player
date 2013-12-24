#include "libraryitemdelegate.h"

#include <cover.h>
#include <settings.h>
#include "librarytreeview.h"
#include "playlists/starrating.h"

#include <memory>

#include <QtDebug>
#include <QImageReader>

using namespace std;

LibraryItemDelegate::LibraryItemDelegate(LibraryFilterProxyModel *proxy) :
	QStyledItemDelegate(proxy)
{
	_proxy = proxy;
	_libraryModel = qobject_cast<QStandardItemModel*>(_proxy->sourceModel());
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStandardItem *item = _libraryModel->itemFromIndex(index);
	QStyleOptionViewItemV4 o = option;
	initStyleOption(&o, index);

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	int type = item->data(LibraryTreeView::Type).toInt();
	if (type == LibraryTreeView::Track && Settings::getInstance()->isStarDelegates()) {
		QString absFilePath = item->data(LibraryTreeView::DataAbsFilePath).toString();
		FileHelper fh(absFilePath);
		qDebug() << "rating" << fh.rating();
		if (fh.rating() > 0) {
			StarRating starRating(fh.rating());
			starRating.paint(painter, o.rect, o.palette, StarRating::ReadOnly);
		}
	}

	switch (type) {
	case LibraryTreeView::Album:
		this->drawAlbum(painter, o, item);
		break;
	case LibraryTreeView::Artist:
		this->drawArtist(painter, o, index);
		break;
	case LibraryTreeView::Disc:
		this->drawDisc(painter, o, index);
		break;
	case LibraryTreeView::Letter:
		this->drawLetter(painter, o, index);
		break;
	case LibraryTreeView::Track:
		this->drawTrack(painter, o, item);
		break;
	default:
		QStyledItemDelegate::paint(painter, o, index);
		break;
	}
}

/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Settings *settings = Settings::getInstance();
	//QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	QStandardItem *item = _libraryModel->itemFromIndex(index);
	int type = item->data(LibraryTreeView::Type).toInt();
	if (settings->isCoversEnabled() && type == LibraryTreeView::Album) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

/** Albums have covers usually. */
void LibraryItemDelegate::drawAlbum(QPainter *painter, const QStyleOptionViewItem &option, QStandardItem *item) const
{
	static QImageReader imageReader;
	QString file = item->data(LibraryTreeView::DataCoverPath).toString();
	if (item->icon().isNull() && !file.isEmpty()) {
		FileHelper fh(file);
		QFileInfo f(file);
		// If it's an inner cover
		if (FileHelper::suffixes().contains(f.suffix())) {
			unique_ptr<Cover> cover(fh.extractCover());
			if (cover) {
				QPixmap p;
				p.loadFromData(cover->byteArray(), cover->format());
				item->setIcon(QIcon(p));
			} else {
				item->setIcon(QIcon());
			}
		} else {
			imageReader.setFileName(QDir::fromNativeSeparators(file));
			imageReader.setScaledSize(QSize(Settings::getInstance()->coverSize(), Settings::getInstance()->coverSize()));
			item->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
		}
	}
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
	int trackNumber = track->data(LibraryTreeView::DataTrackNumber).toInt();
	QString title = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	option.text = title;
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);
}
