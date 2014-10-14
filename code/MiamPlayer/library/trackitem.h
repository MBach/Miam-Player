#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class TrackItem : public QStandardItem
{
public:
	explicit TrackItem(const QString &text);

	virtual int type() const;
};

#endif // TRACKITEM_H
