#include "separatoritem.h"

SeparatorItem::SeparatorItem(const QString &text) :
	QStandardItem(text)
{}

int SeparatorItem::type() const
{
	return LibraryFilterProxyModel::IT_Separator;
}
