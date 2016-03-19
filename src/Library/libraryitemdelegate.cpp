#include "libraryitemdelegate.h"

#include <library/jumptowidget.h>
#include <model/albumdao.h>
#include <styling/imageutils.h>
#include <cover.h>
#include <librarytreeview.h>
#include <settingsprivate.h>
#include <starrating.h>

#include <QApplication>
#include <QDir>
#include <QImageReader>

#include <memory>

#include <QtDebug>

LibraryItemDelegate::LibraryItemDelegate(LibraryTreeView *libraryTreeView, QSortFilterProxyModel *proxy)
	: MiamItemDelegate(proxy)
	, _libraryTreeView(libraryTreeView)
{
	connect(_timer, &QTimer::timeout, this, [=]() {
		_iconOpacity += 0.01;
		_libraryTreeView->viewport()->update();
		if (_iconOpacity >= 1) {
			_timer->stop();
			_iconOpacity = 1.0;
		} else {
			// Restart the timer until r has reached the maximum (1.0)
			_timer->start();
		}
	});

	_coverSize = Settings::instance()->coverSizeLibraryTree();
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	auto settings = SettingsPrivate::instance();
	painter->setFont(settings->font(SettingsPrivate::FF_Library));
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	QStyleOptionViewItem o = option;
	initStyleOption(&o, index);
	o.palette = QApplication::palette();
	if (QGuiApplication::isLeftToRight()) {
		o.rect.adjust(0, 0, -_libraryTreeView->jumpToWidget()->width(), 0);
	} else {
		o.rect.adjust(_libraryTreeView->jumpToWidget()->width(), 0, 0, 0);
	}

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	switch (item->type()) {
	case Miam::IT_Album:
		this->paintRect(painter, o);
		this->drawAlbum(painter, o, item);
		break;
	case Miam::IT_Artist:
		this->paintRect(painter, o);
		this->drawArtist(painter, o, item);
		break;
	case Miam::IT_Disc:
		this->paintRect(painter, o);
		this->drawDisc(painter, o, item);
		break;
	case Miam::IT_Separator:
		this->drawLetter(painter, o, item);
		break;
	case Miam::IT_Track: {
		SettingsPrivate::LibrarySearchMode lsm = settings->librarySearchMode();
		if (Settings::instance()->isCoverBelowTracksEnabled() && ((_proxy->filterRegExp().isEmpty() && lsm == SettingsPrivate::LSM_Filter) ||
				lsm == SettingsPrivate::LSM_HighlightOnly)) {
			this->paintCoverOnTrack(painter, o, item);
		} else {
			this->paintRect(painter, o);
		}
		this->drawTrack(painter, o, item);
		break;
	}
	default:
		QStyledItemDelegate::paint(painter, o, index);
		break;
	}
	painter->restore();
}

/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	Settings *settings = Settings::instance();
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	if (item->type() == Miam::IT_Album) {
		QFontMetrics fmf(settingsPrivate->font(SettingsPrivate::FF_Library));
		return QSize(option.rect.width(), qMax(fmf.height(), settings->coverSizeLibraryTree() + 2));
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

/** Albums have covers usually. */
void LibraryItemDelegate::drawAlbum(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	/// XXX: reload cover with high resolution when one has increased coverSize (every 64px)
	static QImageReader imageReader;
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();

	QString coverPath;
	coverPath = item->data(Miam::DF_CoverPath).toString();
	//qDebug() << Q_FUNC_INFO << item->icon().isNull() << coverPath;
	if (!coverPath.isEmpty() && item->icon().isNull()) {
	//if (!_loadedCovers.contains(item) && !coverPath.isEmpty()) {
		FileHelper fh(coverPath);
		// If it's an inner cover, load it
		if (FileHelper::suffixes().contains(fh.fileInfo().suffix())) {
			//qDebug() << Q_FUNC_INFO << "loading internal cover from file";
			std::unique_ptr<Cover> cover(fh.extractCover());
			//if (cover && p.loadFromData(cover->byteArray(), cover->format())) {
			if (cover) {
				//qDebug() << Q_FUNC_INFO << "cover was extracted";
				QPixmap p;
				if (p.loadFromData(cover->byteArray(), cover->format())) {
					//p = p.scaled(_coverSize, _coverSize);
					if (!p.isNull()) {
						item->setIcon(p);
						//_loadedCovers.insert(item, true);
					}
				} else {
					//qDebug() << Q_FUNC_INFO << "couldn't load data into QPixmap";
				}
			} else {
				//qDebug() << Q_FUNC_INFO << "couldn't extract inner cover";
			}
		} else {
			//qDebug() << Q_FUNC_INFO << "loading external cover from harddrive";
			imageReader.setFileName(QDir::fromNativeSeparators(coverPath));
			imageReader.setScaledSize(QSize(_coverSize, _coverSize));
			item->setIcon(QPixmap::fromImage(imageReader.read()));
			//_loadedCovers.insert(item, true);
		}
	}

	painter->save();
	QRect cover;
	if (QGuiApplication::isLeftToRight()) {
		cover = QRect(option.rect.x() + 1, option.rect.y() + 1, _coverSize, _coverSize);
	} else {
		cover = QRect(option.rect.width() + 19 - _coverSize - 1, option.rect.y() + 1, _coverSize, _coverSize);
	}
	// If font size is greater than the cover, align it
	if (_coverSize < option.rect.height() - 2) {
		painter->translate(0, (option.rect.height() - 1 - _coverSize) / 2);
	}

	if (coverPath.isEmpty()) {
		if (_iconOpacity <= 0.25) {
			painter->setOpacity(_iconOpacity);
		} else {
			painter->setOpacity(0.25);
		}
		painter->drawPixmap(cover, QPixmap(":/icons/disc"));
	} else {
		painter->setOpacity(_iconOpacity);
		QPixmap p = option.icon.pixmap(QSize(_coverSize, _coverSize));
		painter->drawPixmap(cover, p);
	}
	painter->restore();

	// Add an icon on the right if album is from some remote location
	bool isRemote = item->data(Miam::DF_IsRemote).toBool();
	int offsetWidth = 0;
	if (isRemote) {
		int iconSize = 31;
		QRect iconRemoteRect(option.rect.x() + option.rect.width() - (iconSize + 4),
							 (option.rect.height() - iconSize)/ 2 + option.rect.y() + 2,
							 iconSize,
							 iconSize);
		QPixmap iconRemote(item->data(Miam::DF_IconPath).toString());
		painter->save();
		painter->setOpacity(0.5);
		painter->drawPixmap(iconRemoteRect, iconRemote);
		painter->restore();
		offsetWidth = iconSize;
	}

	option.textElideMode = Qt::ElideRight;
	QRect rectText;

	// It's possible to have missing covers in your library, so we need to keep alignment.
	if (QGuiApplication::isLeftToRight()) {
		rectText = QRect(option.rect.x() + _coverSize + 5,
						 option.rect.y(),
						 option.rect.width() - (_coverSize + 7) - offsetWidth,
						 option.rect.height() - 1);
	} else {
		rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - _coverSize - 5, option.rect.height());
	}

	QFontMetrics fmf(settingsPrivate->font(SettingsPrivate::FF_Library));
	QString s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());

	this->paintText(painter, option, rectText, s, item);
}

void LibraryItemDelegate::drawArtist(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	auto settings = SettingsPrivate::instance();
	QFontMetrics fmf(settings->font(SettingsPrivate::FF_Library));
	option.textElideMode = Qt::ElideRight;
	QRect rectText;
	QString s;
	if (QGuiApplication::isLeftToRight()) {
		QPoint topLeft(option.rect.x() + 5, option.rect.y());
		rectText = QRect(topLeft, option.rect.bottomRight());
		QString custom = item->data(Miam::DF_CustomDisplayText).toString();
		if (!custom.isEmpty() && settings->isReorderArtistsArticle()) {
			/// XXX: paint articles like ", the" in gray? Could be nice
			s = fmf.elidedText(custom, Qt::ElideRight, rectText.width());
		} else {
			s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
		}
	} else {
		rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
		s = fmf.elidedText(option.text, Qt::ElideRight, rectText.width());
	}
	this->paintText(painter, option, rectText, s, item);
}

void LibraryItemDelegate::drawDisc(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	option.state = QStyle::State_None;
	QPointF p1 = option.rect.bottomLeft(), p2 = option.rect.bottomRight();
	p1.setX(p1.x() + 2);
	p2.setX(p2.x() - 2);
	painter->setPen(Qt::gray);
	painter->drawLine(p1, p2);
	QStyledItemDelegate::paint(painter, option, item->index());
}

void LibraryItemDelegate::drawTrack(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *track) const
{
	auto settings = Settings::instance();
	if (settings->libraryHasStars()) {
		int r = track->data(Miam::DF_Rating).toInt();
		QStyleOptionViewItem copy(option);
		copy.rect = QRect(0, option.rect.y(), option.rect.x(), option.rect.height());
		/// XXX: create an option to display stars right to the text, and fade them if text is too large
		//copy.rect = QRect(option.rect.x() + option.rect.width() - option.rect.height() * 5, option.rect.y(), option.rect.height() * 5, option.rect.height());

		StarRating starRating(r);
		if (r > 0) {
			starRating.paintStars(painter, copy, StarRating::ReadOnly);
		} else if (settings->isShowNeverScored()) {
			starRating.paintStars(painter, copy, StarRating::NoStarsYet);
		}
	}
	MiamItemDelegate::drawTrack(painter, option, track);
}

void LibraryItemDelegate::paintCoverOnTrack(QPainter *painter, const QStyleOptionViewItem &opt, const QStandardItem *track) const
{
	Settings *settings = Settings::instance();
	const QImage *image = _libraryTreeView->expandedCover(static_cast<AlbumItem*>(track->parent()));
	if (image && !image->isNull()) {
		// Copy QStyleOptionViewItem to be able to expand it to the left, and take the maximum available space
		QStyleOptionViewItem option(opt);
		option.rect.setX(0);

		int totalHeight = track->model()->rowCount(track->parent()->index()) * option.rect.height();
		QImage scaled;
		QRect subRect;
		int row = _proxy->mapFromSource(track->index()).row();
		if (totalHeight > option.rect.width()) {
			scaled = image->scaledToWidth(option.rect.width());
			subRect = option.rect.translated(option.rect.width() - scaled.width(), -option.rect.y() + option.rect.height() * row);
		} else {
			scaled = image->scaledToHeight(totalHeight);
			int dx = option.rect.width() - scaled.width();
			subRect = option.rect.translated(-dx, -option.rect.y() + option.rect.height() * row);
		}

		// Fill with white when there are too much tracks to paint (height of all tracks is greater than the scaled image)
		QImage subImage = scaled.copy(subRect);
		if (scaled.height() < subRect.y() + subRect.height()) {
			subImage.fill(option.palette.base().color());
		}

		painter->save();
		painter->setOpacity(1 - settings->coverBelowTracksOpacity());
		painter->drawImage(option.rect, subImage);

		// Over paint black pixel in white
		QRect t(option.rect.x(), option.rect.y(), option.rect.width() - scaled.width(), option.rect.height());
		QImage white(t.size(), QImage::Format_ARGB32);
		white.fill(option.palette.base().color());
		painter->setOpacity(1.0);
		painter->drawImage(t, white);

		// Create a mix with 2 images: first one is a 3 pixels subimage of the album cover which is expanded to the left border
		// The second one is a computer generated gradient focused on alpha channel
		QImage leftBorder = scaled.copy(0, subRect.y(), 3, option.rect.height());
		if (!leftBorder.isNull()) {

			// Because the expanded border can look strange to one, is blurred with some gaussian function
			leftBorder = leftBorder.scaled(t.width(), option.rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			leftBorder = ImageUtils::blurred(leftBorder, leftBorder.rect(), 10, false);
			painter->setOpacity(1 - settings->coverBelowTracksOpacity());
			painter->drawImage(t, leftBorder);

			QLinearGradient linearAlphaBrush(0, 0, leftBorder.width(), 0);
			linearAlphaBrush.setColorAt(0, QApplication::palette().base().color());
			linearAlphaBrush.setColorAt(1, Qt::transparent);

			painter->setOpacity(1.0);
			painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
			painter->setPen(Qt::NoPen);
			painter->setBrush(linearAlphaBrush);
			painter->drawRect(t);
		}
		painter->restore();
	}

	// Display a light selection rectangle when one is moving the cursor
	painter->save();
	QColor color = opt.palette.highlight().color();
	color.setAlphaF(0.66);
	if (opt.state.testFlag(QStyle::State_MouseOver) && !opt.state.testFlag(QStyle::State_Selected)) {
		painter->setPen(opt.palette.highlight().color());
		painter->setBrush(color.lighter(lighterValue));
		painter->drawRect(opt.rect.adjusted(0, 0, -1, -1));
	} else if (opt.state.testFlag(QStyle::State_Selected)) {
		// Display a not so light rectangle when one has chosen an item. It's darker than the mouse over
		painter->setPen(opt.palette.highlight().color());
		painter->setBrush(color.lighter(150));
		painter->drawRect(opt.rect.adjusted(0, 0, -1, -1));
	}
	painter->restore();
}

/** Check if color needs to be inverted then paint text. */
void LibraryItemDelegate::paintText(QPainter *p, const QStyleOptionViewItem &opt, const QRect &rectText, const QString &text, const QStandardItem *item) const
{
	p->save();
	if (text.isEmpty()) {
		p->setPen(opt.palette.mid().color());
		QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
		p->drawText(rectText, Qt::AlignVCenter, fmf.elidedText(tr("(empty)"), Qt::ElideRight, rectText.width()));
	} else {
		if (opt.state.testFlag(QStyle::State_Selected) || opt.state.testFlag(QStyle::State_MouseOver)) {
			if (qAbs(opt.palette.highlight().color().lighter(lighterValue).value() - opt.palette.highlightedText().color().value()) < 128) {
				p->setPen(opt.palette.text().color());
			} else {
				p->setPen(opt.palette.highlightedText().color());
			}
		}
		if (item->data(Miam::DF_Highlighted).toBool()) {
			QFont f = p->font();
			f.setBold(true);
			p->setFont(f);
		}
		p->drawText(rectText, Qt::AlignVCenter, text);
	}
	p->restore();
}

void LibraryItemDelegate::displayIcon(bool b)
{
	if (b) {
		_timer->start();
	} else {
		_iconOpacity = 0;
	}
}

void LibraryItemDelegate::updateCoverSize()
{
	qDebug() << Q_FUNC_INFO;
	_coverSize = Settings::instance()->coverSizeLibraryTree();
}
