#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QDialog>

#include "ui_playlistmanager.h"

#include "../playlists/tabplaylist.h"

#include <QSqlDatabase>
#include <QStandardItem>

class PlaylistManager : public QDialog, public Ui::PlaylistManager
{
	Q_OBJECT

private:
	TabPlaylist *playlists;

	QMap<int, QStandardItem *> map;

	QSqlDatabase *_db;

public:
	PlaylistManager(QSqlDatabase *db, TabPlaylist *tabPlaylist);

	bool eventFilter(QObject *obj, QEvent *event);

	void init();

	void savePlaylist(int index);

private:
	void loadPreviewPlaylist(QListView *list);

public slots:
	void open();

private slots:
	void clearPlaylist(int i);
	void savePlaylists();
	void updatePlaylists();

	void deleteSavedPlaylists();
	void feedPreviewFromSaved(QItemSelection, QItemSelection);
	void feedPreviewFromUnsaved(QItemSelection, QItemSelection);
	void loadSavedPlaylists();

	void test(const QModelIndex &, int start, int end);

signals:
	void playlistSaved(int);

};

#endif // PLAYLISTMANAGER_H
