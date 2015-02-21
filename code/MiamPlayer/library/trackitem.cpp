#include "trackitem.h"

TrackItem::TrackItem(const TrackDAO *dao) :
	QStandardItem(dao->title())
{
	setData(dao->uri(), LibraryFilterProxyModel::DF_URI);
	setData(dao->titleNormalized(), LibraryFilterProxyModel::DF_NormalizedString);
	setData(dao->trackNumber(), LibraryFilterProxyModel::DF_TrackNumber);
	setData(!dao->uri().startsWith("file://"), LibraryFilterProxyModel::DF_IsRemote);
}

int TrackItem::type() const
{
	return LibraryFilterProxyModel::IT_Track;
}
