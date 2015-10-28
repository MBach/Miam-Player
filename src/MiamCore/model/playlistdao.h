#ifndef PLAYLISTDAO_H
#define PLAYLISTDAO_H

#include "genericdao.h"
#include <QIcon>

/**
 * \brief		The PlaylistDAO class is a simple wrapper which contains basic informations about a playlist.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY PlaylistDAO : public GenericDAO
{
	Q_OBJECT
private:
	QString _background, _length;

public:
	explicit PlaylistDAO(QObject *parent = nullptr);

	PlaylistDAO(const PlaylistDAO &other);

	PlaylistDAO& operator=(const PlaylistDAO& other);

	virtual ~PlaylistDAO();

	QString background() const;
	void setBackground(const QString &background);

	QString length() const;
	void setLength(const QString &length);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(PlaylistDAO)

#endif // PLAYLISTDAO_H
