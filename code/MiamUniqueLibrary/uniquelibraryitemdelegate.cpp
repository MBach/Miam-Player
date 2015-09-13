#include "uniquelibraryitemdelegate.h"

#include <settingsprivate.h>
#include <QApplication>
#include <QPainter>
#include <QStandardItem>

UniqueLibraryItemDelegate::UniqueLibraryItemDelegate(QSortFilterProxyModel *proxy)
	: MiamItemDelegate(proxy)
{

}

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
		//o.rect.adjust(0, 0, -_libraryTreeView->jumpToWidget()->width(), 0);
	} else {
		//o.rect.adjust(_libraryTreeView->jumpToWidget()->width(), 0, 0, 0);
	}

	// Removes the dotted rectangle to the focused item
	o.state &= ~QStyle::State_HasFocus;
	switch (item->type()) {
	case Miam::IT_Album:
		this->paintRect(painter, o);
		painter->drawText(o.rect, item->text());
		break;
	case Miam::IT_Artist:
		this->paintRect(painter, o);
		painter->drawText(o.rect, item->text());
		break;
	case Miam::IT_Disc:
		break;
	case Miam::IT_Separator:
		this->drawLetter(painter, o, static_cast<SeparatorItem*>(item));
		break;
	case Miam::IT_Track: {
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
