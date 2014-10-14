#ifndef DISCITEM_H
#define DISCITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class DiscItem : public QStandardItem
{
public:
	explicit DiscItem(const QString &text);

	virtual int type() const;
};

#endif // DISCITEM_H
