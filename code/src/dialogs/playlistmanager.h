#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QDialog>

#include "ui_playlistmanager.h"

#include "tabplaylist.h"

#include <QStandardItem>

class PlaylistManager : public QDialog, public Ui::PlaylistManager
{
	Q_OBJECT

private:
	TabPlaylist *playlists;

	QMap<int, QStandardItem *> map;

public:
	PlaylistManager(TabPlaylist *tabPlaylist, QWidget *parent = 0);

	bool eventFilter(QObject *obj, QEvent *event);

private:
	void loadPreviewPlaylist(QListView *list);

public slots:
	void open();

private slots:
	void clearPlaylist(int i);
	void updatePlaylists();

	void deleteSavedPlaylists();
	void feedPreviewFromSaved(QItemSelection, QItemSelection);
	void feedPreviewFromUnsaved(QItemSelection, QItemSelection);
	void loadSavedPlaylists();

	void test(const QModelIndex &, int, int);

};

#endif // PLAYLISTMANAGER_H
