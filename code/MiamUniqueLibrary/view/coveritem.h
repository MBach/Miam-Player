#ifndef COVERITEM_H
#define COVERITEM_H

#include <QStandardItem>
#include "miamcore_global.h"

class CoverItem : public QStandardItem
{
public:
	virtual int type() const;
};

#endif // COVERITEM_H
