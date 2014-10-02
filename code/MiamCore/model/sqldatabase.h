#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QSqlDatabase>

#include "miamcore_global.h"
#include "remotetrack.h"
#include "remoteplaylist.h"

/**
 * \brief		The SqlDatabase class uses SQLite to store few but useful tables for tracks, playlists, etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SqlDatabase : public QSqlDatabase
{
public:
	explicit SqlDatabase();

	virtual ~SqlDatabase() {}

	bool insertIntoTablePlaylistTracks(int playlistId, const std::list<RemoteTrack> &tracks);

	int insertIntoTablePlaylists(const RemotePlaylist &playlist);

	void removePlaylists(const QList<RemotePlaylist> &playlists);

	QList<RemoteTrack> selectPlaylistTracks(int playlistID);

	RemotePlaylist selectPlaylist(int playlistId);
	QList<RemotePlaylist> selectPlaylists();
};

#endif // SQLDATABASE_H
