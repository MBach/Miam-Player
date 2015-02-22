#ifndef TABLEFILTERPROXYMODEL_H
#define TABLEFILTERPROXYMODEL_H

#include "../miamuniquelibrary_global.h"

#include <QSortFilterProxyModel>

class MIAMUNIQUELIBRARY_LIBRARY TableFilterProxyModel : public QSortFilterProxyModel
{
public:
	explicit TableFilterProxyModel(QObject *parent = NULL);
};

#endif // TABLEFILTERPROXYMODEL_H
