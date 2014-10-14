#ifndef YEARITEM_H
#define YEARITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class YearItem : public QStandardItem
{
public:
	explicit YearItem(const QString &text);

	virtual int type() const;
};

#endif // YEARITEM_H
