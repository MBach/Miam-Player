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
	p->save();
	p->setPen(o.palette.mid().color());
	if (QApplication::isLeftToRight() && index.column() == 0) {
		p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
	} else if (!QApplication::isLeftToRight() && index.column() == 0) {
		p->drawLine(o.rect.topRight(), o.rect.bottomRight());
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
	if (opt.state.testFlag(QStyle::State_Selected)) {
		p->setPen(opt.palette.highlight().color().darker(150));
		if (settings->isCustomColors()) {
			p->fillRect(o.rect, opt.palette.highlight().color());
		} else {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter());
		}

		// Don't display the upper line is the track above is selected
		QModelIndex top = index.sibling(index.row() - 1, index.column());
		if (!top.isValid() || !_tableView->selectionModel()->selectedIndexes().contains(top)) {
			p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
		}
		if (opt.rect.left() == 0) {
			p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
		} else if (opt.rect.right() == _tableView->viewport()->rect().right()) {
			p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
		}
		// Don't display the bottom line is the track underneath is selected
		QModelIndex bottom = index.sibling(index.row() + 1, index.column());
		if (!bottom.isValid() || !_tableView->selectionModel()->selectedIndexes().contains(bottom)) {
			p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
		}
	} else {
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);
	}
	p->restore();
	if (!_fallback) {
		style->drawItemText(p, o.rect, Qt::AlignLeft | Qt::AlignVCenter, o.palette, true, index.data().toString());
	}
}
