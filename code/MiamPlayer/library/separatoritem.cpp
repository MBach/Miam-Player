#include "separatoritem.h"

SeparatorItem::SeparatorItem(const QString &text) :
	QStandardItem(text)
{}

int SeparatorItem::type() const
{
	return Miam::IT_Separator;
}
