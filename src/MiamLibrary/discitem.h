#ifndef DISCITEM_H
#define DISCITEM_H

#include <QStandardItem>
#include "miamlibrary_global.hpp"

class MIAMLIBRARY_LIBRARY DiscItem : public QStandardItem
{
public:
	explicit DiscItem(const QString &text);

	virtual int type() const override;
};

#endif // DISCITEM_H
