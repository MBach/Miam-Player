#include "separatoritem.h"
#include "miamcore_global.h"

SeparatorItem::SeparatorItem(const QString &text) :
	QStandardItem(text)
{
	setData(text.left(1).toLower(), Miam::DF_NormalizedString);
}

int SeparatorItem::type() const
{
	return Miam::IT_Separator;
}
