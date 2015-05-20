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
	SqlDatabase *_db;

	QLabel *_labelEmptyPreview;

	/** Reference to TabPlaylist used a lot to know what we are manipulating. */
	TabPlaylist *_tabPlaylists;

	/** Display an icon when the Preview Area is empty. */
	QStackedLayout *_stackLayout;

	/** Volatile models in the Dialog to separate which playlists were save or not. */
	QStandardItemModel *_unsavedPlaylistModel, *_savedPlaylistModel;

	Q_ENUMS(PlaylistRoles)

public:
	enum PlaylistRoles { PlaylistObjectPointer	= Qt::UserRole + 1,
						 PlaylistID				= Qt::UserRole + 2};

	explicit PlaylistManager(SqlDatabase *db, TabPlaylist *tabPlaylist);

	/** Add drag & drop processing. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	void init();

	int savePlaylist(int index, bool isOverwriting = false, bool isExitingApplication = false);

	void retranslateUi(PlaylistManager *dialog);

private:
	void clearPreview(bool aboutToInsertItems = true);

	/** Remove all special characters for Windows, Unix, OSX. */
	static QString convertNameToValidFileName(QString &name);

	/** Load a playlist saved on the in database. */
	void loadPlaylist(uint playlistId);

public slots:
	/** Redefined: clean preview area, populate once again lists. */
	void open();

	void deletePlaylist(int index, Playlist *p);

	void saveAndRemovePlaylist(int index, bool isOverwriting = false);

private slots:
	/** Delete from the file system every selected playlists. Cannot be canceled. */
	void deleteSavedPlaylists();

	void dropAutoSavePlaylists(const QModelIndex &, int start, int);

	/** Export one playlist at a time. */
	void exportSelectedPlaylist();

	/** Load every saved playlists. */
	void loadSelectedPlaylists();

	void populatePreviewFromSaved(QItemSelection, QItemSelection);

	void populatePreviewFromUnsaved(QItemSelection, QItemSelection);

	/** Update saved and unsaved playlists when one is adding a new one. Also used at startup. */
	void updatePlaylists(bool unsaved = true, bool saved = true);

signals:
	void aboutToRemovePlaylist(int);
};

#endif // PLAYLISTMANAGER_H
