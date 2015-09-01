#ifndef ALBUMITEM_H
#define ALBUMITEM_H

#include <QStandardItem>
#include "miamcore_global.h"

class AlbumItem : public QStandardItem
{
public:
	virtual int type() const;
};

#endif // ALBUMITEM_H
