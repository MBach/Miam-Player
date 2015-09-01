#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include "miamcore_global.h"

class ArtistItem : public QStandardItem
{
public:
	virtual int type() const;
};

#endif // ARTISTITEM_H
