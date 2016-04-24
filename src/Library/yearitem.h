#ifndef YEARITEM_H
#define YEARITEM_H

#include <QStandardItem>
#include "libraryfilterproxymodel.h"
#include "miamlibrary_global.hpp"

/**
 * \brief		The YearItem class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY YearItem : public QStandardItem
{
public:
	explicit YearItem(const QString &year);

	virtual ~YearItem() {}

	virtual int type() const override;

	uint hash() const;
};

#endif // YEARITEM_H
