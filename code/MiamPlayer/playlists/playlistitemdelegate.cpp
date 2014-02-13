#include "playlistitemdelegate.h"

#include "playlist.h"
#include "settings.h"

#include <QPainter>
#include <QApplication>

#include <QtDebug>

PlaylistItemDelegate::PlaylistItemDelegate(Playlist *playlist) :
	QStyledItemDelegate(playlist), _playlist(playlist)
{}

void PlaylistItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QStyleOptionViewItem o(opt);
	QStyle *style = o.widget ? o.widget->style() : QApplication::style();
	o.state &= ~QStyle::State_HasFocus;
	p->save();
	if (opt.state.testFlag(QStyle::State_Selected) && opt.state.testFlag(QStyle::State_Active) ) {
		p->fillRect(o.rect, opt.palette.highlight().color().lighter());
		p->setPen(opt.palette.highlight().color());

		// Don't display the upper line is the track above is selected
		QModelIndex top = index.sibling(index.row() - 1, index.column());
		if (!top.isValid() || !_playlist->selectionModel()->selectedIndexes().contains(top)) {
			p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
		}
		if (opt.rect.left() == 0) {
			p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
		} else if (opt.rect.right() == _playlist->viewport()->rect().right()) {
			p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
		}
		// Don't display the bottom line is the track underneath is selected
		QModelIndex bottom = index.sibling(index.row() + 1, index.column());
		if (!bottom.isValid() || !_playlist->selectionModel()->selectedIndexes().contains(bottom)) {
			p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
		}
	} else {
		p->setPen(Qt::NoPen);
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);
	}
	p->restore();

	QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &o, o.widget);
	p->setFont(Settings::getInstance()->font(Settings::PLAYLIST));
	style->drawItemText(p, textRect, Qt::AlignLeft | Qt::AlignVCenter, o.palette, true, index.data().toString());
}
