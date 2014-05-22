#include "miamstyleditemdelegate.h"

#include "settings.h"

#include <QApplication>
#include <QPainter>

MiamStyledItemDelegate::MiamStyledItemDelegate(QTableView *parent, bool fallback) :
	QStyledItemDelegate(parent), _tableView(parent), _fallback(fallback)
{
}

/** Redefined. */
void MiamStyledItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QStyleOptionViewItem o(opt);
	initStyleOption(&o, index);
	QStyle *style = QApplication::style();
	o.state &= ~QStyle::State_HasFocus;
	o.palette = QApplication::palette();
	p->save();
	p->setPen(o.palette.mid().color());
	if (_fallback) {
		if (QApplication::isLeftToRight() && index.column() == 0) {
			p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
		} else if (!QApplication::isLeftToRight() && index.column() == 0) {
			p->drawLine(o.rect.topRight(), o.rect.bottomRight());
		}
	}
	Settings *settings = Settings::getInstance();
	// Light color when mouse is over
	/*if (o.state.testFlag(QStyle::State_MouseOver) && !o.state.testFlag(QStyle::State_Selected)) {
		qDebug() << "ici";
		p->setPen(opt.palette.highlight().color().darker(150));
		if (settings->isCustomColors()) {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter(160));
		} else {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter(170));
		}
	} else*/
	if (o.state.testFlag(QStyle::State_Selected)) {
		p->setPen(o.palette.highlight().color().darker(150));
		if (settings->isCustomColors()) {
			p->fillRect(o.rect, o.palette.highlight().color());
		} else {
			p->fillRect(o.rect, o.palette.highlight().color().lighter());
		}

		// Don't display the upper line is the track above is selected
		QModelIndex top = index.sibling(index.row() - 1, index.column());
		if (!top.isValid() || !_tableView->selectionModel()->selectedIndexes().contains(top)) {
			p->drawLine(o.rect.topLeft(), o.rect.topRight());
		}
		if (o.rect.left() == 0) {
			p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
		} else if (o.rect.right() == _tableView->viewport()->rect().right()) {
			p->drawLine(o.rect.topRight(), o.rect.bottomRight());
		}
		// Don't display the bottom line is the track underneath is selected
		QModelIndex bottom = index.sibling(index.row() + 1, index.column());
		if (!bottom.isValid() || !_tableView->selectionModel()->selectedIndexes().contains(bottom)) {
			p->drawLine(o.rect.bottomLeft(), o.rect.bottomRight());
		}
	} else {
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);
	}
	p->restore();
	if (!_fallback) {
		if (o.state.testFlag(QStyle::State_Selected)) {
			p->setPen(o.palette.highlightedText().color());
		} else {
			p->setPen(o.palette.text().color());
		}
		p->drawText(o.rect, Qt::AlignLeft | Qt::AlignVCenter,
					p->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight, o.rect.width()));
	}
}
