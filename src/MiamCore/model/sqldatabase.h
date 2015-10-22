#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include "../miamcore_global.h"
#include "artistdao.h"
#include "albumdao.h"
#include "trackdao.h"
#include "playlistdao.h"
#include "yeardao.h"

#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QThread>
#include <QWeakPointer>

class Cover;
class FileHelper;
class MusicSearchEngine;

/**
 * \brief		The SqlDatabase class uses SQLite to store few but useful tables for tracks, playlists, etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SqlDatabase : public QObject, public QSqlDatabase
{
	Q_OBJECT
private:
	static SqlDatabase *_sqlDatabase;

	SqlDatabase();

	/** This worker is used to avoid a blocking UI when scanning the FileSystem. */
	QThread _workerThread;

	/** Object than can iterate throught the FileSystem for Audio files. */
	MusicSearchEngine *_musicSearchEngine;

	QHash<uint, GenericDAO*> _cache;

	Q_ENUMS(extension)

public:
	/** Singleton pattern to be able to easily use settings everywhere in the app. */
	static SqlDatabase* instance();

	MusicSearchEngine * musicSearchEngine() const;

	bool insertIntoTableArtists(ArtistDAO *artist);
	bool insertIntoTableAlbums(uint artistId, AlbumDAO *album);
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
	AlbumDAO* selectAlbumFromArtist(ArtistDAO *artistDAO, uint albumId);
	TrackDAO selectTrackByURI(const QString &uri);

	bool playlistHasBackgroundImage(uint playlistID);
	bool updateTablePlaylist(const PlaylistDAO &playlist);
	void updateTablePlaylistWithBackgroundImage(uint playlistID, const QString &backgroundImagePath);
	void updateTableAlbumWithCoverImage(const QString &coverPath, const QString &album, const QString &artist);

	/** Update a list of tracks. If track name has changed, it will be removed from Library then added right after. */
	void updateTracks(const QStringList &oldPaths, const QStringList &newPaths);

	QString normalizeField(const QString &s) const;

	void setPragmas();

private:
	/** When one has manually updated tracks with TagEditor, some nodes might in unstable state. */
	bool cleanNodesWithoutTracks();

	/** Read all tracks entries in the database and send them to connected views. */
	void loadFromFileDB(bool sendResetSignal = true);

public slots:
	/** Load an existing database file or recreate it, if not found. */
	void load();

	/** Delete and rescan local tracks. */
	void rebuild();

	void rebuild(const QStringList &oldLocations, const QStringList &newLocations);

private slots:
	/** Reads an external picture which is close to multimedia files (same folder). */
	void saveCoverRef(const QString &coverPath, const QString &track);

	/** Reads a file from the filesystem and adds it into the library. */
	void saveFileRef(const QString &absFilePath);

signals:
	void aboutToLoad();
	void aboutToResyncRemoteSources();
	void coverWasUpdated(const QFileInfo &);
	void loaded();
	void progressChanged(const int &);

	void nodeExtracted(GenericDAO *node);
	void aboutToUpdateNode(GenericDAO *node);

	//void aboutToUpdateView(const QList<FileHelper*> &olds, const QList<FileHelper*> &news);
	void aboutToCleanView();
};

#endif // SQLDATABASE_H
