#include "coveritem.h"

#include <miamcore_global.h>

CoverItem::CoverItem(const QString &coverPath)
	: QStandardItem()
{
	this->setData(coverPath, Miam::DF_CoverPath);
}

int CoverItem::type() const
{
	return Miam::IT_Cover;
}
