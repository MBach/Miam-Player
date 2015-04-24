#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include "model/artistdao.h"

class ArtistItem : public QStandardItem
{
public:
	ArtistItem(const ArtistDAO *dao);

	virtual int type() const override;
};

#endif // ARTISTITEM_H
