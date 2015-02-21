#ifndef YEARITEM_H
#define YEARITEM_H

#include <QStandardItem>
#include "library/libraryfilterproxymodel.h"
#include "model/yeardao.h"

class YearItem : public QStandardItem
{
public:
	explicit YearItem(const YearDAO *dao);

	virtual int type() const;
};

#endif // YEARITEM_H
