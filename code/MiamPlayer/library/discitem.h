#ifndef DISCITEM_H
#define DISCITEM_H

#include <QStandardItem>

class DiscItem : public QStandardItem
{
public:
	explicit DiscItem(const QString &text);

	virtual int type() const override;
};

#endif // DISCITEM_H
