#ifndef ALBUMITEM_H
#define ALBUMITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class AlbumItem : public QStandardItem
{
public:
	explicit AlbumItem(const AlbumDAO *dao);

	virtual int type() const;
};

#endif // ALBUMITEM_H
