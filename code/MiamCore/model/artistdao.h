#ifndef ARTISTDAO_H
#define ARTISTDAO_H

#include "genericdao.h"

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
