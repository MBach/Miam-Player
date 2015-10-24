#ifndef DISCITEM_H
#define DISCITEM_H

#include <QStandardItem>
#include "miamlibrary_global.hpp"

/**
 * \brief		The DiscItem class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY DiscItem : public QStandardItem
{
public:
	explicit DiscItem(const QString &text);

	virtual int type() const override;
};

#endif // DISCITEM_H
