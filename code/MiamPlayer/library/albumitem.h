#ifndef ALBUMITEM_H
#define ALBUMITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class AlbumItem : public QStandardItem
{
public:
	explicit AlbumItem(const QString &text = 0);

	virtual int type() const;
};

#endif // ALBUMITEM_H
