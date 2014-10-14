#ifndef LETTERITEM_H
#define LETTERITEM_H

#include <QStandardItem>
#include "librarytreeview.h"

class LetterItem : public QStandardItem
{
public:
	explicit LetterItem(const QString &text);

	virtual int type() const;
};

#endif // LETTERITEM_H
