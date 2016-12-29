#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QFileInfo>
#include "miamtabplaylists_global.hpp"

/// Forward declarations
class TabPlaylist;
class Playlist;

/**
 * \brief		The PlaylistManager class is used to Create/Read/Update/Delete playlists in SQLite DB.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY PlaylistManager : public QObject
{
	Q_OBJECT
private:
	TabPlaylist *_tabPlaylists;

public:
	explicit PlaylistManager(TabPlaylist *parent);

	bool loadPlaylist(Playlist *p, const QFileInfo &fileInfo);

public slots:
	bool deletePlaylist(uint playlistId);

	uint savePlaylist(Playlist *p, bool isOverwriting, bool isExiting);

	void saveAndRemovePlaylist(Playlist *p, int index, bool isOverwriting = false);

signals:
	void aboutToRemovePlaylist(int);
};

#endif // PLAYLISTMANAGER_H
