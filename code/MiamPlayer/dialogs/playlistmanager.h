#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QDialog>

#include "ui_playlistmanager.h"

#include "../playlists/tabplaylist.h"

#include "model/sqldatabase.h"
#include <QStackedLayout>
#include <QStandardItem>

/**
 * \brief The PlaylistManager class can save, load and export playlists in m3u format.
 */
class PlaylistManager : public QDialog, public Ui::PlaylistManager
{
	Q_OBJECT

private:
	SqlDatabase _db;

	/** Reference to TabPlaylist used a lot to know what we are manipulating. */
	TabPlaylist *playlists;

	/** Display an icon when the Preview Area is empty. */
	QStackedLayout *_stackLayout;

	/** Volatile models in the Dialog to separate which playlists were save or not. */
	QStandardItemModel *_unsavedPlaylistModel, *_savedPlaylistModel;

	Q_ENUMS(PlaylistRoles)

public:
	enum PlaylistRoles { PlaylistPath			= Qt::UserRole + 1,
						 IsPlaylistRegistered	= Qt::UserRole + 2,
						 PlaylistObjectPointer	= Qt::UserRole + 3};

	explicit PlaylistManager(const SqlDatabase &db, TabPlaylist *tabPlaylist);

	bool eventFilter(QObject *obj, QEvent *event);

	void init();

	void saveAndRemovePlaylist(int index);

private:
	void clearPreview(bool aboutToInsertItems = true);

	/** Remove all special characters for Windows, Unix, OSX. */
	static QString convertNameToValidFileName(QString &name);

	QString getPlaylistName(const QString &path);

	/** Load a playlist (*.m3u8) saved on the filesystem. */
	void loadPlaylist(const QString &path);

	bool savePlaylist(int index);

public slots:
	/** Redefined: clean preview area, populate once again lists. */
	void open();

private slots:
	/** Delete from the file system every selected playlists. Cannot be canceled. */
	void deleteSavedPlaylists();

	void dropAutoSavePlaylists(const QModelIndex &parent, int start, int);

	/** Export one playlist at a time. */
	void exportSelectedPlaylist();

	/** Load every saved playlists. */
	void loadSelectedPlaylists();

	void populatePreviewFromSaved(QItemSelection, QItemSelection);

	void populatePreviewFromUnsaved(QItemSelection, QItemSelection);

	/** Save all playlists when exiting the application (if enabled). */
	void savePlaylists();

	/** Update saved and unsaved playlists when one is adding a new one. Also used at startup. */
	void updatePlaylists();

signals:
	void playlistSaved(int);
};

#endif // PLAYLISTMANAGER_H
