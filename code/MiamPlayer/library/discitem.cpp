#include "discitem.h"

DiscItem::DiscItem(const QString &text) :
	QStandardItem(text)
{}

int DiscItem::type() const
{
	return LibraryTreeView::IT_Disc;
}
