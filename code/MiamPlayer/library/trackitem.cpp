#include "trackitem.h"

TrackItem::TrackItem(const QString &text) :
	QStandardItem(text)
{}

int TrackItem::type() const
{
	return LibraryTreeView::IT_Track;
}
