#ifndef SEPARATORITEM_H
#define SEPARATORITEM_H

#include <QStandardItem>

class SeparatorItem : public QStandardItem
{
public:
	explicit SeparatorItem(const QString &text);

	virtual int type() const override;
};

#endif // SEPARATORITEM_H
