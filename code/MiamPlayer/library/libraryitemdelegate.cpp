#include "libraryitemdelegate.h"

#include <cover.h>
#include <settings.h>
#include "librarytreeview.h"
#include "../playlists/starrating.h"

#include <memory>

#include <QtDebug>
#include <QImageReader>

LibraryItemDelegate::LibraryItemDelegate(LibraryFilterProxyModel *proxy) :
	QStyledItemDelegate(proxy), _animateIcons(false), _iconSizeChanged(false), _iconOpacity(1.0)
{
	_proxy = proxy;
	_libraryModel = qobject_cast<QStandardItemModel*>(_proxy->sourceModel());
	_showCovers = Settings::getInstance()->isCoversEnabled();
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	//qDebug() << Q_FUNC_INFO;
	painter->save();
	QStandardItem *item = _libraryModel.data()->itemFromIndex(_proxy.data()->mapToSource(index));
	QStyleOptionViewItem o = option;
	initStyleOption(&o, index);

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	int type = item->data(LibraryTreeView::Type).toInt();
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
	painter->restore();
}

/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Settings *settings = Settings::getInstance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	int type = item->data(LibraryTreeView::Type).toInt();
	if (settings->isCoversEnabled() && type == LibraryTreeView::Album) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

#include "library/iconengine.h"

/** Albums have covers usually. */
void LibraryItemDelegate::drawAlbum(QPainter *painter, const QStyleOptionViewItem &option, QStandardItem *item) const
{
	static QImageReader imageReader;
	int coverSize = Settings::getInstance()->coverSize();
	QString file = item->data(LibraryTreeView::DataCoverPath).toString();
	if (option.state & QStyle::State_MouseOver && ~option.state & QStyle::State_Selected) {
		painter->save();
		qDebug() << option.palette.color(QPalette::Active, QPalette::Highlight).name(QColor::HexArgb);
		painter->setPen(option.palette.highlight().color());
		painter->setBrush(option.palette.highlight().color().lighter(175));
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	} else if (option.state & QStyle::State_Selected) {
		painter->save();
		qDebug() << option.palette.color(QPalette::Active, QPalette::Highlight).name(QColor::HexArgb);
		painter->setPen(option.palette.highlight().color());
		painter->setBrush(option.palette.highlight().color().lighter(160));
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	}
	if (_showCovers) {
		if ((item->icon().isNull() || item->data(Qt::UserRole + 20).toBool() == 1) && !file.isEmpty()) {
			FileHelper fh(file);
			QFileInfo f(file);
			qDebug() << "loading cover from harddrive";
			// If it's an inner cover
			if (FileHelper::suffixes().contains(f.suffix())) {
				std::unique_ptr<Cover> cover(fh.extractCover());
				if (cover) {
					QPixmap p;
					p.loadFromData(cover->byteArray(), cover->format());
					p = p.scaled(coverSize, coverSize);
					item->setIcon(QIcon(p));
				}
			} else {
				imageReader.setFileName(QDir::fromNativeSeparators(file));
				imageReader.setScaledSize(QSize(coverSize, coverSize));
				item->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
			}
		}
	}
	if (_showCovers && item->icon().isNull()) {
		QPixmap pixmap(coverSize, coverSize);
		pixmap.fill(option.palette.alternateBase().color());
		QIcon ico(pixmap);
		ico.addPixmap(pixmap);
		item->setIcon(ico);
		// Mark this item with empty pixmap
		/// XXX: extract somewhere
		item->setData(1, Qt::UserRole + 20);
	}
	if (_showCovers) {
		QPixmap p = option.icon.pixmap(QSize(coverSize, coverSize));
		QRect cover(option.rect.x() + 1, option.rect.y() + 1, coverSize, coverSize);
		if (_animateIcons) {
			painter->save();
			painter->setOpacity(_iconOpacity);
			painter->drawPixmap(cover, p);
			painter->restore();
		} else {
			painter->drawPixmap(cover, p);
		}
	}
	QPoint topLeft(option.rect.x() + coverSize, option.rect.y());
	QRect rectCoverSize(topLeft, option.rect.bottomRight());
	painter->drawText(rectCoverSize, option.text);
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
	if (Settings::getInstance()->isStarDelegates()) {
		QString absFilePath = track->data(LibraryTreeView::DataAbsFilePath).toString();
		/// XXX: query the sqlmodel instead?
		FileHelper fh(absFilePath);
		//qDebug() << "rating" << fh.rating();
		if (fh.rating() > 0) {
			StarRating starRating(fh.rating());
			starRating.paint(painter, option.rect, option.palette, StarRating::ReadOnly);
		}
	}
	int trackNumber = track->data(LibraryTreeView::DataTrackNumber).toInt();
	QString title = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	option.text = title;
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);
}

void LibraryItemDelegate::displayIcon(bool b)
{
	qDebug() << Q_FUNC_INFO;
	if (b) {
		_showCovers = true;
		_animateIcons = true;
	} else {
		this->setIconOpacity(0);
	}
}
