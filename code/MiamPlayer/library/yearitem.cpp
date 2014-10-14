#include "yearitem.h"

YearItem::YearItem(const QString &text) :
	QStandardItem(text)
{}

int YearItem::type() const
{
	return LibraryTreeView::IT_Year;
}
