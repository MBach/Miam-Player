#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QStandardItem>
#include "model/trackdao.h"
#include "miamlibrary_global.hpp"

/**
 * \brief		The TrackItem class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY TrackItem : public QStandardItem
{
public:
	explicit TrackItem(const TrackDAO *dao);

	virtual ~TrackItem() {}

	virtual int type() const override;
};

#endif // TRACKITEM_H
