#ifndef SEPARATORITEM_H
#define SEPARATORITEM_H

#include <QStandardItem>
#include "library/libraryfilterproxymodel.h"

class SeparatorItem : public QStandardItem
{
public:
	explicit SeparatorItem(const QString &text);

	virtual int type() const;
};

#endif // SEPARATORITEM_H
