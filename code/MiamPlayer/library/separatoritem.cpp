#include "Separatoritem.h"

SeparatorItem::SeparatorItem(const QString &text) :
	QStandardItem(text)
{}

int SeparatorItem::type() const
{
	return LibraryTreeView::IT_Separator;
}
