#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QSqlDatabase>

#include "miamcore_global.h"
#include "trackdao.h"
#include "playlistdao.h"

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

	bool insertIntoTablePlaylistTracks(int playlistId, const std::list<TrackDAO> &tracks);

	int insertIntoTablePlaylists(const PlaylistDAO &playlist);

	void removePlaylists(const QList<PlaylistDAO> &playlists);

	QList<TrackDAO> selectPlaylistTracks(int playlistID);

	PlaylistDAO selectPlaylist(int playlistId);
	QList<PlaylistDAO> selectPlaylists();
};

#endif // SQLDATABASE_H
