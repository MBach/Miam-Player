#ifndef SEPARATORITEM_H
#define SEPARATORITEM_H

#include <QStandardItem>
#include "miamlibrary_global.hpp"

/**
 * \brief		The SeparatorItem class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY SeparatorItem : public QStandardItem
{
public:
	explicit SeparatorItem(const QString &text);

	virtual ~SeparatorItem() {}

	virtual int type() const override;
};

#endif // SEPARATORITEM_H
