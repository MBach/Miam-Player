#ifndef YEARITEM_H
#define YEARITEM_H

#include <QStandardItem>
#include <model/yeardao.h>
#include "libraryfilterproxymodel.h"
#include "miamlibrary_global.h"

class MIAMLIBRARY_LIBRARY YearItem : public QStandardItem
{
public:
	explicit YearItem(const YearDAO *dao);

	virtual int type() const override;
};

#endif // YEARITEM_H
