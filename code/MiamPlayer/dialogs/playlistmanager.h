#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QDialog>

#include "ui_playlistmanager.h"

#include "../playlists/tabplaylist.h"

#include <QSqlDatabase>
#include <QStackedLayout>
#include <QStandardItem>

/**
 * \brief The PlaylistManager class can save, load and export playlists in m3u format.
 */
class PlaylistManager : public QDialog, public Ui::PlaylistManager
{
	Q_OBJECT

private:
	TabPlaylist *playlists;

	QSqlDatabase _db;

	QStackedLayout *_stackLayout;

public:
	explicit PlaylistManager(const QSqlDatabase &db, TabPlaylist *tabPlaylist);

	bool eventFilter(QObject *obj, QEvent *event);

	void init();

	void saveAndRemovePlaylist(int index);

private:
	void clearPreview(bool aboutToInsertItems = true);

	QString getPlaylistName(const QString &path);

	void loadPlaylist(const QString &path);

	bool savePlaylist(int index);

public slots:
	/** Redefined: clean preview area, populate once again lists. */
	void open();

private slots:
	void deleteSavedPlaylists();

	void dropAutoSavePlaylists(const QModelIndex &, int start, int end);

	void feedPreviewFromSaved(QItemSelection, QItemSelection);

	void feedPreviewFromUnsaved(QItemSelection, QItemSelection);

	/** Load every selected playlists. */
	void loadSavedPlaylists();

	/** Save all playlists when exiting the application (if enabled). */
	void savePlaylists();

	/** Update saved and unsaved playlists when one is adding a new one. Also used at startup. */
	void updatePlaylists();

signals:
	void playlistSaved(int);
};

#endif // PLAYLISTMANAGER_H
