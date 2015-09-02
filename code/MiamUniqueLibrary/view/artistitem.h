#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include <model/artistdao.h>
#include <miamcore_global.h>

class ArtistItem : public QStandardItem
{
public:
	ArtistItem(const GenericDAO *dao);

	virtual int type() const;
};

#endif // ARTISTITEM_H
