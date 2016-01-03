#include "artistitem.h"
#include "miamcore_global.h"

#include <QRegularExpression>

ArtistItem::ArtistItem(const ArtistDAO *dao)
	: QStandardItem(dao->title())
{
	if (dao->titleNormalized().isEmpty() || !dao->titleNormalized().contains(QRegularExpression("[\\w]"))) {
		setData("0", Miam::DF_NormalizedString);
	} else {
		setData(dao->titleNormalized(), Miam::DF_NormalizedString);
	}
	setData(dao->id(), Miam::DF_ID);
	setData(dao->customData(), Miam::DF_CustomDisplayText);
}

int ArtistItem::type() const
{
	return Miam::IT_Artist;
}
