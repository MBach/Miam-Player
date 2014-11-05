#ifndef ARTISTDAO_H
#define ARTISTDAO_H

#include "genericdao.h"

/**
 * \brief		The ArtistDAO class is a simple wrapper.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY ArtistDAO : public GenericDAO
{
	Q_OBJECT
public:
	explicit ArtistDAO(QObject *parent = 0);

	ArtistDAO(const ArtistDAO &remoteArtist);

	virtual ~ArtistDAO();
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(ArtistDAO)


#endif // ARTISTDAO_H
