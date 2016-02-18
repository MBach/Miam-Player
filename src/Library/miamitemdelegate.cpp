#include "miamitemdelegate.h"

#include <settings.h>
#include <settingsprivate.h>
#include <QGuiApplication>
#include <QPainter>

qreal MiamItemDelegate::_iconOpacity = 1.0;

MiamItemDelegate::MiamItemDelegate(QSortFilterProxyModel *proxy)
	: QStyledItemDelegate(proxy)
	, _proxy(proxy)
	, _timer(new QTimer(this))
{
	auto settings = Settings::instance();
	_coverSize = settings->coverSize();
	_showCovers = settings->isCoversEnabled();
	_libraryModel = qobject_cast<QStandardItemModel*>(_proxy->sourceModel());
	_timer->setTimerType(Qt::PreciseTimer);
	_timer->setInterval(10);
}

void MiamItemDelegate::drawLetter(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const
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

void MiamItemDelegate::drawTrack(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *track) const
{
	int trackNumber = track->data(Miam::DF_TrackNumber).toInt();
	if (trackNumber > 0) {
		option.text = QString("%1").arg(trackNumber, 2, 10, QChar('0')).append(". ").append(track->text());
	} else {
		option.text = track->text();
	}
	option.textElideMode = Qt::ElideRight;
	QString s;
	QRect rectText;
	if (QGuiApplication::isLeftToRight()) {
		QPoint topLeft(option.rect.x() + 5, option.rect.y());
		rectText = QRect(topLeft, option.rect.bottomRight());
		s = painter->fontMetrics().elidedText(option.text, Qt::ElideRight, rectText.width());
	} else {
		rectText = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
		s = painter->fontMetrics().elidedText(option.text, Qt::ElideRight, rectText.width());
	}
	this->paintText(painter, option, rectText, s, track);
}

void MiamItemDelegate::paintRect(QPainter *painter, const QStyleOptionViewItem &option) const
{
	// Display a light selection rectangle when one is moving the cursor
	if (option.state.testFlag(QStyle::State_MouseOver) && !option.state.testFlag(QStyle::State_Selected)) {
		painter->save();
		painter->setPen(option.palette.highlight().color());
		painter->setBrush(option.palette.highlight().color().lighter(160));
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	} else if (option.state.testFlag(QStyle::State_Selected)) {
		// Display a not so light rectangle when one has chosen an item. It's darker than the mouse over
		painter->save();
		painter->setPen(option.palette.highlight().color());
		painter->setBrush(option.palette.highlight().color().lighter(150));
		painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
		painter->restore();
	}
}

/** Check if color needs to be inverted then paint text. */
void MiamItemDelegate::paintText(QPainter *p, const QStyleOptionViewItem &opt, const QRect &rectText, const QString &text, const QStandardItem *item) const
{
	p->save();
	if (text.isEmpty()) {
		p->setPen(opt.palette.mid().color());
		QFontMetrics fmf(SettingsPrivate::instance()->font(SettingsPrivate::FF_Library));
		p->drawText(rectText, Qt::AlignVCenter, p->fontMetrics().elidedText(tr("(empty)"), Qt::ElideRight, rectText.width()));
	} else {
		if (opt.state.testFlag(QStyle::State_Selected) || opt.state.testFlag(QStyle::State_MouseOver)) {
			if (qAbs(opt.palette.highlight().color().lighter(160).value() - opt.palette.highlightedText().color().value()) < 128) {
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
