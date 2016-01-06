#ifndef UNIQUELIBRARYITEMMODEL_H
#define UNIQUELIBRARYITEMMODEL_H

#include "miamuniquelibrary_global.hpp"
#include <miamitemmodel.h>
#include "uniquelibraryfilterproxymodel.h"
#include <model/trackdao.h>
#include <model/albumdao.h>
#include <model/artistdao.h>

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
	explicit UniqueLibraryItemModel(QObject *parent = nullptr);

	virtual QChar currentLetter(const QModelIndex &index) const override;

	virtual UniqueLibraryFilterProxyModel* proxy() const override;

public slots:
	void insertTracks(const QList<TrackDAO> nodes);

	void insertAlbums(const QList<AlbumDAO> nodes);

	void insertArtists(const QList<ArtistDAO> nodes);

private:
	void insertSeparators();

signals:
	void aboutToMergeGrid();
};

#endif // UNIQUELIBRARYITEMMODEL_H
