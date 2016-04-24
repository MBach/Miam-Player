#ifndef ARTISTITEM_H
#define ARTISTITEM_H

#include <QStandardItem>
#include "miamlibrary_global.hpp"

/**
 * \brief		The ArtistItem class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY ArtistItem : public QStandardItem
{
public:
	explicit ArtistItem();

	virtual ~ArtistItem() {}

	virtual int type() const override;

	uint hash() const;
};

#endif // ARTISTITEM_H
