#include "trackitem.h"

TrackItem::TrackItem(const TrackDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->uri(), LibraryTreeView::DF_URI);
	setData(dao->titleNormalized(), LibraryTreeView::DF_NormalizedString);
	setData(dao->trackNumber(), LibraryTreeView::DF_TrackNumber);
	setData(!dao->uri().startsWith("file://"), LibraryTreeView::DF_IsRemote);
}

int TrackItem::type() const
{
	return LibraryTreeView::IT_Track;
}
