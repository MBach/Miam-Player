#include "libraryitemdelegate.h"

#include <cover.h>
#include <settingsprivate.h>
#include "librarytreeview.h"
#include "../playlists/starrating.h"
#include "../model/albumdao.h"

#include <QApplication>
#include <QDir>
#include <QImageReader>

#include <memory>

#include <QtDebug>

LibraryItemDelegate::LibraryItemDelegate(LibraryTreeView *libraryTreeView, LibraryFilterProxyModel *proxy) :
	QStyledItemDelegate(proxy), _animateIcons(false), _iconOpacity(1.0), _libraryTreeView(libraryTreeView)
{
	_proxy = proxy;
	_libraryModel = qobject_cast<QStandardItemModel*>(_proxy->sourceModel());
	_showCovers = SettingsPrivate::instance()->isCoversEnabled();
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	painter->setFont(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
	QStandardItem *item = _libraryModel.data()->itemFromIndex(_proxy.data()->mapToSource(index));
	QStyleOptionViewItem o = option;
	initStyleOption(&o, index);
	o.palette = QApplication::palette();
	if (QGuiApplication::isLeftToRight()) {
		o.rect.adjust(0, 0, -20, 0);
	} else {
		o.rect.adjust(19, 0, 0, 0);
	}

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	switch (item->type()) {
	case LibraryFilterProxyModel::IT_Album:
		this->paintRect(painter, o);
		this->drawAlbum(painter, o, static_cast<AlbumItem*>(item));
		break;
	case LibraryFilterProxyModel::IT_Artist:
		this->paintRect(painter, o);
		this->drawArtist(painter, o, static_cast<ArtistItem*>(item));
		break;
	case LibraryFilterProxyModel::IT_Disc:
		this->paintRect(painter, o);
		this->drawDisc(painter, o, static_cast<DiscItem*>(item));
		break;
	case LibraryFilterProxyModel::IT_Separator:
		this->drawLetter(painter, o, static_cast<SeparatorItem*>(item));
		break;
	case LibraryFilterProxyModel::IT_Track:
		this->paintRect(painter, o);
		this->drawTrack(painter, o, static_cast<TrackItem*>(item));
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
	SettingsPrivate *settings = SettingsPrivate::instance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	if (settings->isCoversEnabled() && item->type() == LibraryFilterProxyModel::IT_Album) {
		QFontMetrics fmf(settings->font(SettingsPrivate::FF_Library));
		return QSize(option.rect.width(), qMax(fmf.height(), settings->coverSize() + 2));
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

/** Albums have covers usually. */
void LibraryItemDelegate::drawAlbum(QPainter *painter, QStyleOptionViewItem &option, AlbumItem *item) const
{
	/// XXX: reload cover with high resolution when one has increased coverSize (every 64px)
	static QImageReader imageReader;
	SettingsPrivate *settings = SettingsPrivate::instance();
	int coverSize = settings->coverSize();
	bool noCoverAvailable = false;
	if (settings->isCoversEnabled()) {
		QString file = item->data(LibraryFilterProxyModel::DF_CoverPath).toString();
		// Qt::UserRole + 20 == false => pixmap not loaded ; == true => pixmap loaded
		/// XXX: extract this elsewhere
		if (item->data(Qt::UserRole + 20).toBool() == false && !file.isEmpty()) {
			FileHelper fh(file);
			// If it's an inner cover, load it
			if (FileHelper::suffixes().contains(fh.fileInfo().suffix())) {
				qDebug() << Q_FUNC_INFO << "loading internal cover from file";
				std::unique_ptr<Cover> cover(fh.extractCover());
				QPixmap p;
				if (cover && p.loadFromData(cover->byteArray(), cover->format())) {
					p = p.scaled(coverSize, coverSize);
					if (!p.isNull()) {
						item->setIcon(p);
						item->setData(true, Qt::UserRole + 20);
					}
				}
			} else {
				qDebug() << Q_FUNC_INFO << "loading external cover from harddrive";
				imageReader.setFileName(QDir::fromNativeSeparators(file));
				imageReader.setScaledSize(QSize(coverSize, coverSize));
				item->setIcon(QPixmap::fromImage(imageReader.read()));
				item->setData(true, Qt::UserRole + 20);
			}
		} else if (file.isEmpty()) {
			noCoverAvailable = true;
			item->setData(true, Qt::UserRole + 20);
		}
	} else {
		item->setIcon(QIcon());
		item->setData(false, Qt::UserRole + 20);
	}
	bool b = item->data(Qt::UserRole + 20).toBool();
	if (settings->isCoversEnabled() && b) {
		QPixmap p = option.icon.pixmap(QSize(coverSize, coverSize));
		QRect cover;
		if (QGuiApplication::isLeftToRight()) {
			cover = QRect(option.rect.x() + 1, option.rect.y() + 1, coverSize, coverSize);
		} else {
			cover = QRect(option.rect.width() + 19 - coverSize - 1, option.rect.y() + 1, coverSize, coverSize);
		}
		// If font size is greater than the cover, align it
		painter->save();
		if (coverSize < option.rect.height() - 2) {
			painter->translate(0, (option.rect.height() - 1 - coverSize) / 2);
		}
		if (_animateIcons) {
			painter->save();
			painter->setOpacity(_iconOpacity);
			painter->drawPixmap(cover, p);
			painter->restore();
		} else {
			if (noCoverAvailable) {
				painter->setOpacity(0.25);
				QPixmap p2(":/icons/disc");
				painter->drawPixmap(cover, p2);
			} else {
				painter->drawPixmap(cover, p);
			}
		}
		painter->restore();
	}

	// Add an icon on the right if album is from some remote location
	bool isRemote = item->data(LibraryFilterProxyModel::DF_IsRemote).toBool();
	int offsetWidth = 0;
	if (isRemote) {
		int iconSize = 31;
		QRect iconRemoteRect(option.rect.x() + option.rect.width() - (iconSize + 4),
							 (option.rect.height() - iconSize)/ 2 + option.rect.y() + 2,
							 iconSize,
							 iconSize);
		QPixmap iconRemote(item->iconPath());
		painter->save();
		painter->setOpacity(0.5);
		painter->drawPixmap(iconRemoteRect, iconRemote);
		painter->restore();
		offsetWidth = iconSize;
	}

	QFontMetrics fmf(settings->font(SettingsPrivate::FF_Library));
	option.textElideMode = Qt::ElideRight;
	QString s;
	QRect rectText;
	if (settings->isCoversEnabled()) {
		// It's possible to have missing covers in your library, so we need to keep alignment.
		if (QGuiApplication::isLeftToRight()) {
			rectText = QRect(option.rect.x() + coverSize + 5,
							 option.rect.y(),
							 option.rect.width() - (coverSize + 7) - offsetWidth,
							 option.rect.height() - 1);
		} else {
			rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - coverSize - 5, option.rect.height());
		}
	} else {
		rectText = QRect(option.rect.x() + 5, option.rect.y(), option.rect.width() - 5, option.rect.height());
	}
	s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	this->paintText(painter, option, rectText, s, item);
}

void LibraryItemDelegate::drawArtist(QPainter *painter, QStyleOptionViewItem &option, ArtistItem *item) const
{
	QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
	option.textElideMode = Qt::ElideRight;
	QRect rectText;
	QString s;
	if (QGuiApplication::isLeftToRight()) {
		QPoint topLeft(option.rect.x() + 5, option.rect.y());
		rectText = QRect(topLeft, option.rect.bottomRight());
		s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	} else {
		rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
		s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	}
	this->paintText(painter, option, rectText, s, item);
}

void LibraryItemDelegate::drawDisc(QPainter *painter, QStyleOptionViewItem &option, DiscItem *item) const
{
	option.state = QStyle::State_None;
	QPointF p1 = option.rect.bottomLeft(), p2 = option.rect.bottomRight();
	p1.setX(p1.x() + 2);
	p2.setX(p2.x() - 2);
	painter->setPen(Qt::gray);
	painter->drawLine(p1, p2);
	QStyledItemDelegate::paint(painter, option, item->index());
}

void LibraryItemDelegate::drawLetter(QPainter *painter, QStyleOptionViewItem &option, SeparatorItem *item) const
{
	// One cannot interact with an alphabetical separator
	option.state = QStyle::State_None;
	option.font.setBold(true);
	QPointF p1 = option.rect.bottomLeft(), p2 = option.rect.bottomRight();
	p1.setX(p1.x() + 2);
	p2.setX(p2.x() - 2);
	painter->setPen(Qt::gray);
	painter->drawLine(p1, p2);
	QStyledItemDelegate::paint(painter, option, item->index());
}

void LibraryItemDelegate::drawTrack(QPainter *painter, QStyleOptionViewItem &option, const QStandardItem *track) const
{
	/// XXX: it will be a piece of cake to add an option that one can customize how track number will be displayed
	/// QString title = settings->libraryItemTitle();
	/// for example: zero padding
	if (SettingsPrivate::instance()->isStarDelegates()) {
		QString absFilePath = track->data(LibraryFilterProxyModel::DF_URI).toString();
		/// XXX: query the sqlmodel instead?
		FileHelper fh(absFilePath);
		if (fh.rating() > 0) {
			//StarRating starRating(fh.rating());
			//starRating.paint(painter, option, StarRating::ReadOnly);
		}
	}
	int trackNumber = track->data(LibraryFilterProxyModel::DF_TrackNumber).toInt();
	if (trackNumber > 0) {
		option.text = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	} else {
		option.text = track->text();
	}
	QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
	option.textElideMode = Qt::ElideRight;
	QString s;
	QRect rectText;
	if (QGuiApplication::isLeftToRight()) {
		QPoint topLeft(option.rect.x() + 5, option.rect.y());
		rectText = QRect(topLeft, option.rect.bottomRight());
		s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	} else {
		rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
		s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	}
	this->paintText(painter, option, rectText, s, track);
}

void LibraryItemDelegate::paintRect(QPainter *painter, const QStyleOptionViewItem &option) const
{
	// Display a light selection rectangle when one is moving the cursor
	if (option.state.testFlag(QStyle::State_MouseOver) && !option.state.testFlag(QStyle::State_Selected)) {
		painter->save();
		if (SettingsPrivate::instance()->isCustomColors()) {
			painter->setPen(option.palette.highlight().color().darker(100));
			painter->setBrush(option.palette.highlight().color().lighter());
		} else {
			painter->setPen(option.palette.highlight().color());
			painter->setBrush(option.palette.highlight().color().lighter(170));
		}
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	} else if (option.state.testFlag(QStyle::State_Selected)) {
		// Display a not so light rectangle when one has chosen an item. It's darker than the mouse over
		painter->save();
		if (SettingsPrivate::instance()->isCustomColors()) {
			painter->setPen(option.palette.highlight().color().darker(150));
			painter->setBrush(option.palette.highlight().color());
		} else {
			painter->setPen(option.palette.highlight().color());
			painter->setBrush(option.palette.highlight().color().lighter(160));
		}
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	}
}

/** Check if color needs to be inverted then paint text. */
void LibraryItemDelegate::paintText(QPainter *p, const QStyleOptionViewItem &opt, const QRect &rectText, const QString &text, const QStandardItem *item) const
{
	p->save();
	if (opt.state.testFlag(QStyle::State_Selected) || opt.state.testFlag(QStyle::State_MouseOver)) {
		if ((opt.palette.highlight().color().lighter(160).saturation() - opt.palette.highlightedText().color().saturation()) < 128) {
			p->setPen(opt.palette.text().color());
		} else {
			p->setPen(opt.palette.highlightedText().color());
		}
	}
	if (item->data(LibraryFilterProxyModel::DF_Highlighted).toBool()) {
		QFont f = p->font();
		f.setBold(true);
		p->setFont(f);
	}
	p->drawText(rectText, Qt::AlignVCenter, text);
	p->restore();
}

void LibraryItemDelegate::displayIcon(bool b)
{
	_showCovers = b;
	if (_showCovers) {
		_animateIcons = true;
	} else {
		this->setIconOpacity(0);
	}
}
