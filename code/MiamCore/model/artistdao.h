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
private:
	QString _customData;

public:
	explicit ArtistDAO(QObject *parent = 0);

	ArtistDAO(const ArtistDAO &remoteArtist);

	void setCustomData(const QString &data);
	QString customData() const;

	virtual ~ArtistDAO();

	virtual uint hash() const;
};

//#include <QHash>
//#include <QtDebug>

/*inline bool operator ==(const ArtistDAO &a1, const ArtistDAO &a2)
{
	qDebug() << Q_FUNC_INFO;
	return a1.title() == a2.title() && a1.type() == a2.type();
}

inline uint qHash(const ArtistDAO &key, uint seed)
{
	qDebug() << Q_FUNC_INFO;
	return qHash(key.title(), seed) ^ qHash(key.type(), seed);
}

inline uint qHash(const ArtistDAO *key, uint seed)
{
	qDebug() << Q_FUNC_INFO;
	return qHash(key->title(), seed) ^ qHash(key->type(), seed);
}*/

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(ArtistDAO)

#endif // ARTISTDAO_H
