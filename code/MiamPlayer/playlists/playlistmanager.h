#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>

class TabPlaylist;

class PlaylistManager : public QObject
{
	Q_OBJECT
private:
	TabPlaylist *_tabPlaylists;

public:
	explicit PlaylistManager(TabPlaylist *parent);

	/** Load a playlist saved on the in database. */
	//void loadPlaylist(uint playlistId);

	int savePlaylist(int index, bool isOverwriting, bool isExiting);

public slots:
	void saveAndRemovePlaylist(int index, bool isOverwriting = false, bool isExiting = false);

signals:
	void aboutToRemovePlaylist(int);
};

#endif // PLAYLISTMANAGER_H
