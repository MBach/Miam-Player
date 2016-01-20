#include "yearitem.h"

YearItem::YearItem(const YearDAO *dao) :
	QStandardItem(dao->title())
{
	if (QString::compare(dao->title(), "0") == 0) {
		setText(QObject::tr("Unknown"));
	}
	setData(dao->titleNormalized(), Miam::DF_NormalizedString);
}

int YearItem::type() const
{
	return Miam::IT_Year;
}
