#ifndef ALBUMITEM_H
#define ALBUMITEM_H

#include <QStandardItem>
#include <model/albumdao.h>

#include "miamlibrary_global.hpp"

class MIAMLIBRARY_LIBRARY AlbumItem : public QStandardItem
{
public:
	explicit AlbumItem(const AlbumDAO *dao);

	QString coverPath() const;

	QString iconPath() const;

	virtual int type() const override;
};

#endif // ALBUMITEM_H
