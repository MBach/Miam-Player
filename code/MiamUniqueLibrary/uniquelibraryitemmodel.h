#ifndef UNIQUELIBRARYITEMMODEL_H
#define UNIQUELIBRARYITEMMODEL_H

#include "miamuniquelibrary_global.h"
#include <miamitemmodel.h>
#include <miamsortfilterproxymodel.h>

/**
 * \brief		The UniqueLibraryItemModel class is the model used to store all tracks in a list view.
 * \details		This class is populated from SqlDatabase where all relevant informations are gathered together:
 *				A track is related to Artist, Album, Year so we can sort them nicely and draw cover albums.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryItemModel : public MiamItemModel
{
	Q_OBJECT
private:
	MiamSortFilterProxyModel *_proxy;

public:
	explicit UniqueLibraryItemModel(QObject *parent = 0);

	virtual QChar currentLetter(const QModelIndex &index) const override;

	virtual MiamSortFilterProxyModel* proxy() const override;

public slots:
	virtual void insertNode(GenericDAO *node) override;
};

#endif // UNIQUELIBRARYITEMMODEL_H
