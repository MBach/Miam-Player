#ifndef PLAYLISTDIALOG_H
#define PLAYLISTDIALOG_H

#include <QDialog>

#include "ui_playlistdialog.h"

#include "../playlists/tabplaylist.h"

#include "model/sqldatabase.h"
#include <QStackedLayout>
#include <QStandardItem>

/**
 * \brief The PlaylistDialog class can save, load and export playlists in m3u format.
 */
class PlaylistDialog : public QDialog, public Ui::PlaylistDialog
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

	explicit PlaylistDialog(SqlDatabase *db, TabPlaylist *tabPlaylist);

	/** Add drag & drop processing. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	void init();

	int savePlaylist(int index, bool isOverwriting = false, bool isExiting = false);

	void retranslateUi(PlaylistDialog *dialog);

private:
	void clearPreview(bool aboutToInsertItems = true);

	/** Remove all special characters for Windows, Unix, OSX. */
	static QString convertNameToValidFileName(QString &name);

	/** Load a playlist saved on the in database. */
	void loadPlaylist(uint playlistId);

public slots:
	/** Redefined: clean preview area, populate once again lists. */
	void open();

	void deletePlaylist(int index);

	void saveAndRemovePlaylist(int index, bool isOverwriting = false, bool isExiting = false);

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

#endif // PLAYLISTDIALOG_H
