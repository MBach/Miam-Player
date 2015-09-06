#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QStandardItem>
#include "model/trackdao.h"
#include "miamlibrary_global.h"

class MIAMLIBRARY_LIBRARY TrackItem : public QStandardItem
{
public:
	explicit TrackItem(const TrackDAO *dao);

	virtual int type() const override;
};

#endif // TRACKITEM_H
