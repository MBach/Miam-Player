#ifndef PLAYLISTDIALOG_H
#define PLAYLISTDIALOG_H

#include <QDialog>
#include <QStackedLayout>
#include <QStandardItem>

#include "ui_playlistdialog.h"

#include <model/sqldatabase.h>
#include "../playlists/tabplaylist.h"

/**
 * \brief		The PlaylistDialog class can save, load and export playlists in m3u format.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class PlaylistDialog : public QDialog, public Ui::PlaylistDialog
{
	Q_OBJECT

private:
	QLabel *_labelEmptyPreview;

	/** Display an icon when the Preview Area is empty. */
	QStackedLayout *_stackLayout;

	/** Volatile models in the Dialog to separate which playlists were save or not. */
	QStandardItemModel *_unsavedPlaylistModel, *_savedPlaylistModel;

	QList<Playlist*> _playlists;
	QMap<QStandardItem*, Playlist*> _unsaved;
	QMap<QStandardItem*, PlaylistDAO> _saved;

	Q_ENUMS(PlaylistRoles)

public:
	enum PlaylistRoles { PlaylistID			= Qt::UserRole + 1,
						 PlaylistModified	= Qt::UserRole + 2};

	explicit PlaylistDialog(QWidget *parent = nullptr);

	inline void setPlaylists(const QList<Playlist*> &playlists) { _playlists = playlists; }

	/** Add drag & drop processing. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
	void clearPreview(bool aboutToInsertItems = true);

	/** Remove all special characters for Windows, Unix, OSX. */
	static QString convertNameToValidFileName(const QString &name);

public slots:
	/** Redefined: clean preview area, populate once again lists. */
	void open();

private slots:
	/** Delete from the file system every selected playlists. Cannot be canceled. */
	void deleteSavedPlaylists();

	void dropAutoSavePlaylists(const QModelIndex &, int start, int);

	/** Export one playlist at a time. */
	void exportSelectedPlaylist();

	/** Load every saved playlists. */
	void loadSelectedPlaylists();

	void populatePreviewFromSaved(const QItemSelection &, const QItemSelection &);

	void populatePreviewFromUnsaved(const QItemSelection &, const QItemSelection &);

	void renameItem(QStandardItem *item);

	/** Update saved playlists when one is adding a new one. Also used at startup. */
	void updatePlaylists();

signals:
	void aboutToLoadPlaylist(uint playlistId);
	void aboutToSavePlaylist(Playlist *playlist, bool overwrite);
	void aboutToRenamePlaylist(Playlist *playlist);
	void aboutToRenameTab(const PlaylistDAO &dao);
	void aboutToDeletePlaylist(uint playlistId);
};

#endif // PLAYLISTDIALOG_H
