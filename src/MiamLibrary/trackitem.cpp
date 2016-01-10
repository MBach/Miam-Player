#include "trackitem.h"
#include "miamcore_global.h"

TrackItem::TrackItem(const TrackDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->uri(), Miam::DF_URI);
	setData(dao->titleNormalized(), Miam::DF_NormalizedString);
	setData(dao->trackNumber(), Miam::DF_TrackNumber);
	setData(dao->disc().toInt(), Miam::DF_DiscNumber);
	setData(dao->length(), Miam::DF_TrackLength);
	if (dao->rating() != -1) {
		setData(dao->rating(), Miam::DF_Rating);
	}
	setData(dao->artist(), Miam::DF_Artist);
	setData(dao->album(), Miam::DF_Album);
	setData(!dao->host().isEmpty(), Miam::DF_IsRemote);
}

int TrackItem::type() const
{
	return Miam::IT_Track;
}
