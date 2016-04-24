#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include "../miamcore_global.h"
#include "artistdao.h"
#include "albumdao.h"
#include "settings.h"
#include "trackdao.h"
#include "playlistdao.h"
#include "yeardao.h"

#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QThread>
#include <QUrl>

/// Forward declarations
class Cover;
class FileHelper;

/**
 * \brief		The SqlDatabase class uses SQLite to store few but useful tables for tracks, playlists, etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SqlDatabase : public QObject, public QSqlDatabase
{
	Q_OBJECT
private:
	QHash<uint, GenericDAO*> _cache;

public:
	explicit SqlDatabase();

	~SqlDatabase();

	void init();

	uint insertIntoTablePlaylists(const PlaylistDAO &playlist, const std::list<TrackDAO> &tracks, bool isOverwriting);
	bool insertIntoTablePlaylistTracks(uint playlistId, const std::list<TrackDAO> &tracks, bool isOverwriting = false);
	bool insertIntoTableTracks(const TrackDAO &track);
	bool insertIntoTableTracks(const std::list<TrackDAO> &tracks);

	bool removePlaylist(uint playlistId);
	void removePlaylistsFromHost(const QString &host);
	void removeRecordsFromHost(const QString &host);

	Cover *selectCoverFromURI(const QString &uri);
	QList<TrackDAO> selectPlaylistTracks(uint playlistID);
	PlaylistDAO selectPlaylist(uint playlistId);
	QList<PlaylistDAO> selectPlaylists();

	ArtistDAO* selectArtist(uint artistId);
	TrackDAO selectTrackByURI(const QString &uri);

	bool playlistHasBackgroundImage(uint playlistID);
	bool updateTablePlaylist(const PlaylistDAO &playlist);
	void updateTablePlaylistWithBackgroundImage(uint playlistID, const QString &backgroundImagePath);
	void updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist);

	/** Update a list of tracks. If track name has changed, it will be removed from Library then added right after. */
	void updateTracks(const QStringList &oldPaths, const QStringList &newPaths);

	QString normalizeField(const QString &s) const;

	void setPragmas();

public slots:
	/** Delete cache and rescan local tracks. */
	void rebuild();

	/** Reads an external picture which is close to multimedia files (same folder). */
	void saveCoverRef(const QString &coverPath, const QString &track);

	/** Reads a file from the filesystem and adds it into the library. */
	void saveFileRef(const QString &absFilePath);

signals:
	void aboutToResyncRemoteSources();
	void coverWasUpdated(const QFileInfo &);

	void nodeExtracted(GenericDAO *node);
	void aboutToUpdateNode(GenericDAO *node);

	void aboutToUpdateView(const QList<QUrl> &oldTracks, const QList<QUrl> &newTracks);
	void aboutToCleanView();
};

#endif // SQLDATABASE_H
