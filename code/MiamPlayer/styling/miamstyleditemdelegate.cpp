#include "miamstyleditemdelegate.h"

#include "settingsprivate.h"

#include <QApplication>
#include <QPainter>

#include <QtDebug>

#include <QTreeView>
#include <QTableView>

MiamStyledItemDelegate::MiamStyledItemDelegate(QAbstractItemView *parent, bool fallback) :
	QStyledItemDelegate(parent), _itemView(parent), _fallback(fallback)
{
	/*
	connect(_itemView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &, const QItemSelection &) {
		/// XXX: how to bypass protected method?
		_itemView->setDirtyRegion(QRegion(_itemView->viewport()->rect()));
	});
	*/
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
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (o.state.testFlag(QStyle::State_Selected)) {
		if (settings->isCustomColors()) {
			p->setPen(o.palette.highlight().color().darker(100));
		} else {
			p->setPen(o.palette.highlight().color());
		}
		p->fillRect(o.rect, o.palette.highlight().color().lighter());

		/*QBrush brush = o.palette.highlight();
		QColor color = brush.color().lighter();
		color.setAlpha(100);
		brush.setColor(color);
		p->fillRect(o.rect, brush);*/

		// Don't display the upper line is the track above is selected
		QModelIndex top = index.sibling(index.row() - 1, index.column());
		if (!top.isValid() || !_itemView->selectionModel()->selectedIndexes().contains(top)) {
			p->drawLine(o.rect.topLeft(), o.rect.topRight());
		}
		// Table -> displays only left border for first column, right border for last column
		// Tree -> displays both left and right borders
		if (qobject_cast<QTreeView*>(_itemView)) {
			p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
			p->drawLine(o.rect.topRight(), o.rect.bottomRight());
		} else {
			if (o.rect.left() == 0) {
				//p->drawLine(o.rect.x() + 1, o.rect.y(), o.rect.x() + 1, o.rect.bottom());
				p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
			} else if (o.rect.right() == _itemView->viewport()->rect().right()) {
				p->drawLine(o.rect.topRight(), o.rect.bottomRight());
			}
		}

		// Don't display the bottom line is the track underneath is selected
		QModelIndex bottom = index.sibling(index.row() + 1, index.column());
		if (!bottom.isValid() || !_itemView->selectionModel()->selectedIndexes().contains(bottom)) {
			p->drawLine(o.rect.bottomLeft(), o.rect.bottomRight());
		}
	} else {
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);
	}
	p->restore();
	if (!_fallback) {
		if (o.state.testFlag(QStyle::State_Selected)) {
			if ((o.palette.highlight().color().lighter(160).saturation() - o.palette.highlightedText().color().saturation()) < 128) {
				p->setPen(o.palette.text().color());
			} else {
				p->setPen(o.palette.highlightedText().color());
			}
		} else {
			p->setPen(o.palette.text().color());
		}
		if (o.icon.isNull()) {
			p->drawText(o.rect, Qt::AlignLeft | Qt::AlignVCenter,
						p->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight, o.rect.width()));
		} else {
			// Display icon (file system tree for example)
			QPixmap pm(o.icon.pixmap(18, 18));
			QRect iconRect(o.rect.x() + 1, o.rect.y() + 2, 18, 18);
			p->drawPixmap(iconRect, pm);
			p->drawText(o.rect.adjusted(iconRect.width() + 1, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
						p->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight, o.rect.width()));
		}
	}
}
