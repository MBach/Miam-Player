#include "uniquelibraryitemdelegate.h"

#include <settings.h>
#include <settingsprivate.h>
#include <discitem.h>
#include <QApplication>
#include <QDateTime>
#include <QImageReader>
#include <QPainter>
#include <QStandardItem>

#include <memory>
#include <cover.h>
#include <QDir>

#include <QtDebug>

UniqueLibraryItemDelegate::UniqueLibraryItemDelegate(TableView *tableView)
	: MiamItemDelegate(tableView->model()->proxy())
	, _jumpTo(tableView->jumpToWidget())
{}

void UniqueLibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	/// Work In Progress

	if (index.column() == 0) {
		QString internalCover = index.data(Miam::DF_InternalCover).toString();
		QString cover = index.data(Miam::DF_CoverPath).toString();
		if (!internalCover.isEmpty()) {
			this->drawCover(painter, option, internalCover);
		} else if (!cover.isEmpty()) {
			this->drawCover(painter, option, cover);
		}
		return;
	}
	painter->setFont(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
	QStandardItem *item = _libraryModel->itemFromIndex(_proxy->mapToSource(index));
	QStyleOptionViewItem o = option;
	initStyleOption(&o, index);
	o.palette = QApplication::palette();
	if (QGuiApplication::isLeftToRight()) {
		o.rect.adjust(0, 0, -_jumpTo->width(), 0);
	} else {
		o.rect.adjust(-17, 0, 0, 0);
	}

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	switch (item->type()) {
	case Miam::IT_Artist:
		o.rect.setX(0);
		this->paintRect(painter, o);
		this->drawArtist(painter, o, item);
		break;
	case Miam::IT_Album:
		o.rect.adjust(20, 0, 0, 0);
		this->paintRect(painter, o);
		this->drawAlbum(painter, o, item);
		break;
	case Miam::IT_Disc:
		o.rect.adjust(30, 0, 0, 0);
		this->paintRect(painter, o);
		this->drawDisc(painter, o, item);
		break;
	case Miam::IT_Separator:
		this->drawLetter(painter, o, item);
		break;
	case Miam::IT_Track:
		o.rect.adjust(40, 0, 0, 0);
		this->paintRect(painter, o);
		this->drawTrack(painter, o, item);
		break;
	default:
		QStyledItemDelegate::paint(painter, o, index);
		break;
	}
}

void UniqueLibraryItemDelegate::drawAlbum(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	option.rect.adjust(5, 0, 0, 0);
	QPoint c = option.rect.center();

	QString text = item->text();
	QString year = item->data(Miam::DF_Year).toString();
	if (!year.isEmpty() && (year.compare("0") != 0)) {
		text.append(" [" + item->data(Miam::DF_Year).toString() + "]");
	}
	painter->save();
	QStyle *style = QApplication::style();
	QPalette::ColorRole cr = this->getColorRole(option);
	if (text.isEmpty()) {
		text = tr("(no title)");
		QColor color = QApplication::palette().text().color();
		color.setAlphaF(0.5);
		painter->setPen(color);
	} else if (year.isEmpty()){
		//style->drawItemText(painter, option.rect, Qt::AlignVCenter, option.palette, true, text, cr);
	} else {
		text = painter->fontMetrics().elidedText(text, Qt::ElideRight, option.rect.width());
	}
	style->drawItemText(painter, option.rect, Qt::AlignVCenter, option.palette, true, text, cr);
	painter->restore();
	int textWidth = painter->fontMetrics().width(text);
	painter->drawLine(option.rect.x() + textWidth + 5, c.y(), option.rect.right() - 5, c.y());
}

void UniqueLibraryItemDelegate::drawArtist(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	QStyle *style = QApplication::style();
	QPalette::ColorRole cr = this->getColorRole(option);
	style->drawItemText(painter, option.rect, Qt::AlignVCenter, option.palette, true, item->text(), cr);

	QPoint c = option.rect.center();
	int textWidth = painter->fontMetrics().width(item->text());
	painter->drawLine(option.rect.x() + textWidth + 5, c.y(), option.rect.right() - 5, c.y());
}

void UniqueLibraryItemDelegate::drawCover(QPainter *painter, const QStyleOptionViewItem &option, const QString &coverPath) const
{
	static QImageReader imageReader;
	int coverSize = Settings::instance()->coverSizeUniqueLibrary();

	QRect r(option.rect.x(), option.rect.y(), coverSize, coverSize);

	FileHelper fh(coverPath);
	// If it's an inner cover, load it
	if (FileHelper::suffixes().contains(fh.fileInfo().suffix())) {
		std::unique_ptr<Cover> cover(fh.extractCover());
		if (cover) {
			QPixmap p;
			if (p.loadFromData(cover->byteArray(), cover->format()) && !p.isNull()) {
				painter->drawPixmap(r, p);
			}
		}
	} else {
		imageReader.setFileName(QDir::fromNativeSeparators(coverPath));
		imageReader.setScaledSize(QSize(coverSize, coverSize));
	}
	painter->drawImage(r, imageReader.read());
}

void UniqueLibraryItemDelegate::drawDisc(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
{
	QPoint c = option.rect.center();

	QString text = tr("Disc");
	text.append(" ").append(item->text());
	painter->drawText(option.rect, Qt::AlignVCenter, text);

	int textWidth = painter->fontMetrics().width(text);
	painter->drawLine(option.rect.x() + textWidth + 5, c.y(), option.rect.right() - 5, c.y());
}

void UniqueLibraryItemDelegate::drawTrack(QPainter *p, QStyleOptionViewItem &option, QStandardItem *track) const
{
	p->save();
	int trackNumber = track->data(Miam::DF_TrackNumber).toInt();
	if (trackNumber > 0) {
		option.text = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	} else {
		option.text = track->text();
	}
	option.textElideMode = Qt::ElideRight;
	QString trackLength = QDateTime::fromTime_t(track->data(Miam::DF_TrackLength).toUInt()).toString("m:ss");

	QFont f = SettingsPrivate::instance()->font(SettingsPrivate::FF_Library);
	// Current track is being played
	if (track->data(Miam::DF_Highlighted).toBool()) {
		uint currentPos = track->data(Miam::DF_CurrentPosition).toUInt();
		QString trackCurrentPos = QDateTime::fromTime_t(currentPos).toString("m:ss");
		trackLength.prepend(trackCurrentPos + " / ");
		f.setBold(true);
		p->setFont(f);
	}

	QRect titleRect, lengthRect;
	QString s;

	static int rightIndent = 5;
	int w = p->fontMetrics().width(trackLength);

	if (QGuiApplication::isLeftToRight()) {

		lengthRect = QRect(option.rect.x() + option.rect.width() - (w + rightIndent), option.rect.y(), w + rightIndent, option.rect.height());
		titleRect = QRect(option.rect.x() + rightIndent, option.rect.y(), option.rect.width() - lengthRect.width() - rightIndent, option.rect.height());
		s = p->fontMetrics().elidedText(option.text, Qt::ElideRight, titleRect.width());

	} else {

		lengthRect = QRect(option.rect.x() + option.rect.width() - (w + rightIndent), option.rect.y(), w + rightIndent, option.rect.height());
		titleRect = QRect(option.rect.x(), option.rect.y(), option.rect.width() - rightIndent, option.rect.height());
		s = p->fontMetrics().elidedText(option.text, Qt::ElideRight, titleRect.width());

	}

	// Draw track number and title
	QStyle *style = QApplication::style();
	QPalette::ColorRole cr = this->getColorRole(option);
	if (s.isEmpty()) {
		style->drawItemText(p, titleRect, Qt::AlignVCenter, option.palette, false, p->fontMetrics().elidedText(tr("(empty)"), Qt::ElideRight, titleRect.width()));
	} else {
		style->drawItemText(p, titleRect, Qt::AlignVCenter, option.palette, true, s, cr);
	}

	// Draw track length
	if (QGuiApplication::isLeftToRight()) {
		style->drawItemText(p, lengthRect, Qt::AlignVCenter, option.palette, true, trackLength, cr);
	} else {

	}
	p->restore();
}

QPalette::ColorRole UniqueLibraryItemDelegate::getColorRole(QStyleOptionViewItem &option) const
{
	QPalette::ColorRole cr;
	if (option.state.testFlag(QStyle::State_Selected)) {
		if (qAbs(option.palette.highlight().color().lighter(lighterValue).value() - option.palette.highlightedText().color().value()) < 128) {
			if (SettingsPrivate::instance()->isCustomTextColorOverriden()) {
				cr = QPalette::HighlightedText;
			} else {
				cr = QPalette::Text;
			}
		} else {
			cr = QPalette::HighlightedText;
		}
	} else {
		cr = QPalette::Text;
	}
	return cr;
}
