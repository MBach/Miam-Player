#include "yearitem.h"

YearItem::YearItem(const YearDAO *dao) :
	QStandardItem(dao->title())
{
	if (dao->title().isEmpty()) {
		setText(QObject::tr("Unknown"));
	}
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
}

int YearItem::type() const
{
	return LibraryTreeView::IT_Year;
}
