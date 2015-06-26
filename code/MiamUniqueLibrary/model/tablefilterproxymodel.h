#ifndef TABLEFILTERPROXYMODEL_H
#define TABLEFILTERPROXYMODEL_H

#include "../miamuniquelibrary_global.h"

#include <QSortFilterProxyModel>

class MIAMUNIQUELIBRARY_LIBRARY TableFilterProxyModel : public QSortFilterProxyModel
{
private:
	Q_ENUMS(DataField)

	// User defined data types (item->setData(QVariant, Field);)
	enum DataField { /*DF_URI					= Qt::UserRole + 1,
					 DF_CoverPath			= Qt::UserRole + 2,
					 DF_TrackNumber			= Qt::UserRole + 3,
					 DF_DiscNumber			= Qt::UserRole + 4,*/
					 DF_NormalizedString	= Qt::UserRole + 5/*,
					 DF_Year				= Qt::UserRole + 6,
				   /// TEST QSortFilterProxyModel
					 DF_Highlighted			= Qt::UserRole + 7,
					 DF_IsRemote			= Qt::UserRole + 8,
					 DF_IconPath			= Qt::UserRole + 9*/};
public:
	explicit TableFilterProxyModel(QObject *parent = nullptr);

protected:
	/** Redefined for custom sorting. */
	virtual bool lessThan(const QModelIndex &idxLeft, const QModelIndex &idxRight) const;
};

#endif // TABLEFILTERPROXYMODEL_H
