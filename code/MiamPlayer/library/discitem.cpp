#include "discitem.h"

DiscItem::DiscItem(const QString &text) :
	QStandardItem(text)
{}

int DiscItem::type() const
{
	return Miam::IT_Disc;
}
