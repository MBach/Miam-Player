#include "artistitem.h"

#include <QRegularExpression>

ArtistItem::ArtistItem(const ArtistDAO *dao)
	: QStandardItem(dao->title())
{
	if (dao->titleNormalized().isEmpty() || !dao->titleNormalized().contains(QRegularExpression("[\\w]"))) {
		setData("0", Miam::DF_NormalizedString);
	} else {
		setData(dao->titleNormalized(), Miam::DF_NormalizedString);
	}
}

int ArtistItem::type() const
{
	return Miam::IT_Artist;
}
