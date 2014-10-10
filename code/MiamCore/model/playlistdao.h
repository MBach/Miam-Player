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
	QString _background, _iconPath, _length, _title;

public:
	explicit PlaylistDAO(QObject *parent = 0);

	PlaylistDAO(const PlaylistDAO &remotePlaylist);

	virtual ~PlaylistDAO();

	QString iconPath() const;
	void setIconPath(const QString &icon);

	QString background() const;
	void setBackground(const QString &background);

	QString length() const;
	void setLength(const QString &length);

	QString title() const;
	void setTitle(const QString &title);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(PlaylistDAO)

#endif // PLAYLISTDAO_H
