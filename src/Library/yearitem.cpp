#include "yearitem.h"

YearItem::YearItem(const QString &year)
{
	if (year.isEmpty()) {
		setText(QObject::tr("Unknown"));
	} else {
		setText(year);
	}
	setData(year, Miam::DF_NormalizedString);
}

int YearItem::type() const
{
	return Miam::IT_Year;
}

uint YearItem::hash() const
{
	uint h;
	if (text().isEmpty()) {
		h = 0x9e3779b9;
	} else {
		h = qHash(text(), 0x9e3779b9);
	}
	return h;
}
