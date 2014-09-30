#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include "remoteobject.h"

/**
 * \brief		The RemotePlaylist class is a simple wrapper which contains basic informations about a playlist.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY RemotePlaylist : public RemoteObject
{
	Q_OBJECT
private:
	QString _length, _title;

public:
	explicit RemotePlaylist(QObject *parent = 0);

	RemotePlaylist(const RemotePlaylist &remotePlaylist);

	virtual ~RemotePlaylist();

	QString length() const;
	void setLength(const QString &length);

	QString title() const;
	void setTitle(const QString &title);
};

#endif // REMOTEPLAYLIST_H
