#ifndef COVERITEM_H
#define COVERITEM_H

#include "miamuniquelibrary_global.hpp"
#include <QStandardItem>

class MIAMUNIQUELIBRARY_LIBRARY CoverItem : public QStandardItem
{
public:
	CoverItem(const QString &coverPath);

	virtual int type() const override;
};

#endif // COVERITEM_H
