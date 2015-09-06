#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include <model/artistdao.h>

#include "miamlibrary_global.h"

class MIAMLIBRARY_LIBRARY ArtistItem : public QStandardItem
{
public:
	ArtistItem(const ArtistDAO *dao);

	virtual int type() const override;
};

#endif // ARTISTITEM_H
