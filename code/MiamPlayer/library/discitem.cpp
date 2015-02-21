#include "discitem.h"

DiscItem::DiscItem(const QString &text) :
	QStandardItem(text)
{}

int DiscItem::type() const
{
	return LibraryFilterProxyModel::IT_Disc;
}
