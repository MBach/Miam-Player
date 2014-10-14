#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class ArtistItem : public QStandardItem
{
public:
	ArtistItem(const QString &text);

	virtual int type() const;
};

#endif // ARTISTITEM_H
