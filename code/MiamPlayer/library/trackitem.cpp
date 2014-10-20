#include "trackitem.h"

TrackItem::TrackItem(const TrackDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->trackNumber(), LibraryTreeView::DF_TrackNumber);
	setData(QVariant::fromValue(*dao), LibraryTreeView::DF_DAO);
}

int TrackItem::type() const
{
	return LibraryTreeView::IT_Track;
}
