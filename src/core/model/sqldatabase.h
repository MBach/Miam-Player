#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include "../miamcore_global.h"
#include "settings.h"
#include "trackdao.h"
#include "playlistdao.h"

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
	explicit SqlDatabase(QObject *parent = nullptr);

	~SqlDatabase();

	void reset();

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

	TrackDAO selectTrackByURI(const QString &uri);

	bool playlistHasBackgroundImage(uint playlistID);
	bool updateTablePlaylist(const PlaylistDAO &playlist);
	void updateTablePlaylistWithBackgroundImage(uint playlistID, const QString &backgroundImagePath);
	void updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist);

	/** Update a list of tracks. If track name has changed, it will be removed from Library then added right after. */
	void updateTracks(const QStringList &oldPaths, const QStringList &newPaths);

	QString normalizeField(const QString &s) const;

private:
	void init();

	void setPragmas();

	void updateTrack(const QString &absFilePath);

public slots:
	/** Reads an external picture which is close to multimedia files (same folder). */
	void saveCoverRef(const QString &coverPath, const QString &track);

	/** Reads a file from the filesystem and adds it into the library. */
	void saveFileRef(const QString &absFilePath);

signals:
	void aboutToUpdateView();
};

#endif // SQLDATABASE_H
