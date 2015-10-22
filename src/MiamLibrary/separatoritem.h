#ifndef SEPARATORITEM_H
#define SEPARATORITEM_H

#include <QStandardItem>
#include "miamlibrary_global.hpp"

class MIAMLIBRARY_LIBRARY SeparatorItem : public QStandardItem
{
public:
	explicit SeparatorItem(const QString &text);

	virtual int type() const override;
};

#endif // SEPARATORITEM_H
