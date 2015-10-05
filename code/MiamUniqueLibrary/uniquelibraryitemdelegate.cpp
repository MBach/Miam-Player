#include "uniquelibraryitemdelegate.h"

#include <settingsprivate.h>
#include <QApplication>
#include <QPainter>
#include <QStandardItem>

#include <QtDebug>

UniqueLibraryItemDelegate::UniqueLibraryItemDelegate(JumpToWidget *jumpTo, QSortFilterProxyModel *proxy)
	: MiamItemDelegate(proxy)
	, _jumpTo(jumpTo)
{}

void UniqueLibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	/// Work In Progress

	painter->save();
	auto settings = SettingsPrivate::instance();
	painter->setFont(settings->font(SettingsPrivate::FF_Library));
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	QStyleOptionViewItem o = option;
	initStyleOption(&o, index);
	o.palette = QApplication::palette();
	if (QGuiApplication::isLeftToRight()) {
		o.rect.adjust(0, 0, -_jumpTo->width(), 0);
	} else {
		o.rect.adjust(_jumpTo->width(), 0, 0, 0);
	}

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	switch (item->type()) {
	case Miam::IT_Artist:
		this->paintRect(painter, o);
		this->drawArtist(painter, o, static_cast<ArtistItem*>(item));
		break;
	case Miam::IT_Album:
		this->paintRect(painter, o);
		this->drawAlbum(painter, o, static_cast<AlbumItem*>(item));
		break;
	case Miam::IT_Disc:
		//this->drawDisc(painter, o);
		break;
	case Miam::IT_Separator:
		this->drawLetter(painter, o, static_cast<SeparatorItem*>(item));
		break;
	case Miam::IT_Track: {
		int coverSize = settings->coverSize();
		o.rect.adjust(coverSize, 0, 0, 0);
		this->paintRect(painter, o);
		this->drawTrack(painter, o, static_cast<TrackItem*>(item));
		break;
	}
	default:
		QStyledItemDelegate::paint(painter, o, index);
		break;
	}
	painter->restore();
}

void UniqueLibraryItemDelegate::drawAlbum(QPainter *painter, QStyleOptionViewItem &option, AlbumItem *item) const
{
	auto settings = SettingsPrivate::instance();
	int coverSize = settings->coverSize();
	option.rect.moveLeft(coverSize);
	QString text = item->text();
	QString year = item->data(Miam::DF_Year).toString();
	if (!year.isEmpty() && (year.compare("0") != 0)) {
		text.append(" [" + item->data(Miam::DF_Year).toString() + "]");
	}
	painter->drawText(option.rect, text);
	QPoint c = option.rect.center();
	int textWidth = painter->fontMetrics().width(text);
	painter->drawLine(coverSize + textWidth + 5, c.y(), option.rect.right() - 5, c.y());

	// Cover
	QString coverPath = item->coverPath();
	if (!coverPath.isEmpty()) {
		//qDebug() << Q_FUNC_INFO << coverPath;
	}
}

void UniqueLibraryItemDelegate::drawArtist(QPainter *painter, QStyleOptionViewItem &option, ArtistItem *item) const
{
	painter->drawText(option.rect, item->text());
	QPoint c = option.rect.center();
	int textWidth = painter->fontMetrics().width(item->text());
	painter->drawLine(textWidth + 5, c.y(), option.rect.right() - 5, c.y());
}

#include <QDateTime>

void UniqueLibraryItemDelegate::drawTrack(QPainter *p, QStyleOptionViewItem &option, TrackItem *track) const
{
	int trackNumber = track->data(Miam::DF_TrackNumber).toInt();
	if (trackNumber > 0) {
		option.text = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	} else {
		option.text = track->text();
	}
	QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
	option.textElideMode = Qt::ElideRight;
	QString s;
	QString trackLength = QDateTime::fromTime_t(track->data(Miam::DF_TrackLength).toUInt()).toString("m:ss");
	QRect titleRect, lengthRect;
	if (QGuiApplication::isLeftToRight()) {

		int w = fmf.width(trackLength);
		lengthRect = QRect(option.rect.x() + option.rect.width() - (w + 5), option.rect.y(), w + 5, option.rect.height());
		titleRect = QRect(option.rect.x() + 5, option.rect.y(), option.rect.width() - lengthRect.width() - 5, option.rect.height());

		s = fmf.elidedText(option.text, Qt::ElideRight, titleRect.width());
	} else {
		titleRect = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
		s = fmf.elidedText(option.text, Qt::ElideRight, titleRect.width());
	}

	p->save();
	// Draw track number and title
	if (s.isEmpty()) {
		p->setPen(option.palette.mid().color());
		QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
		p->drawText(titleRect, Qt::AlignVCenter, fmf.elidedText(tr("(empty)"), Qt::ElideRight, titleRect.width()));
	} else {
		if (option.state.testFlag(QStyle::State_Selected) || option.state.testFlag(QStyle::State_MouseOver)) {
			if (qAbs(option.palette.highlight().color().lighter(160).value() - option.palette.highlightedText().color().value()) < 128) {
				p->setPen(option.palette.text().color());
			} else {
				p->setPen(option.palette.highlightedText().color());
			}
		}
		if (track->data(Miam::DF_Highlighted).toBool()) {
			QFont f = p->font();
			f.setBold(true);
			p->setFont(f);
		}
		p->drawText(titleRect, Qt::AlignVCenter, s);
	}

	// Draw track length
	if (QGuiApplication::isLeftToRight()) {
		p->drawText(lengthRect, Qt::AlignVCenter, trackLength);
	} else {

	}
	p->restore();
}
