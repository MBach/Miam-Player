#include "trackitem.h"

TrackItem::TrackItem(const TrackDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->uri(), LibraryTreeView::DF_URI);
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->trackNumber(), LibraryTreeView::DF_TrackNumber);
}

int TrackItem::type() const
{
	return LibraryTreeView::IT_Track;
}
