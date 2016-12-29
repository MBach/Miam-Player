#include "discitem.h"
#include "miamcore_global.h"

DiscItem::DiscItem()
{
	/*setData(dao->titleNormalized(), Miam::DF_NormalizedString);
	setData(dao->artist(), Miam::DF_Artist);
	setData(dao->id(), Miam::DF_ID);
	setData(dao->disc(), Miam::DF_DiscNumber);*/
}

int DiscItem::type() const
{
	return Miam::IT_Disc;
}
