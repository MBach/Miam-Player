#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include "library/libraryfilterproxymodel.h"
#include "model/artistdao.h"

class ArtistItem : public QStandardItem
{
public:
	ArtistItem(const ArtistDAO *dao);

	virtual int type() const;
};

#endif // ARTISTITEM_H
