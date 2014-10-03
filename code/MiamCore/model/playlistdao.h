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
	QString _length, _title;
	QIcon _icon;

public:
	explicit PlaylistDAO(QObject *parent = 0);

	PlaylistDAO(const PlaylistDAO &remotePlaylist);

	virtual ~PlaylistDAO();

	QIcon icon() const;
	void setIcon(const QIcon &icon);

	QString length() const;
	void setLength(const QString &length);

	QString title() const;
	void setTitle(const QString &title);
};

#endif // PLAYLISTDAO_H
