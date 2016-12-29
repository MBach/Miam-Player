#include "coveritem.h"

#include <miamcore_global.h>

CoverItem::CoverItem()
	: QStandardItem()
{

}

int CoverItem::type() const
{
	return Miam::IT_Cover;
}
