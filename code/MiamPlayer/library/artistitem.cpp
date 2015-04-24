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
	if (!dao->customData().isEmpty()) {
		setData(dao->customData(), Miam::DF_Custom);
	}
}

int ArtistItem::type() const
{
	return Miam::IT_Artist;
}
