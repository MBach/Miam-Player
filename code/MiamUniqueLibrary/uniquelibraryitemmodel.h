#ifndef UNIQUELIBRARYITEMMODEL_H
#define UNIQUELIBRARYITEMMODEL_H

#include <miamitemmodel.h>
#include "miamuniquelibrary_global.h"
#include "uniquelibraryfilterproxymodel.h"

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
	UniqueLibraryFilterProxyModel *_proxy;

public:
	explicit UniqueLibraryItemModel(QObject *parent = 0);

	virtual QChar currentLetter(const QModelIndex &index) const override;

	virtual UniqueLibraryFilterProxyModel* proxy() const override;

public slots:
	virtual void insertNode(GenericDAO *node) override;
};

#endif // UNIQUELIBRARYITEMMODEL_H
