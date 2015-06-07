#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>

class TabPlaylist;
class Playlist;

class PlaylistManager : public QObject
{
	Q_OBJECT
private:
	TabPlaylist *_tabPlaylists;

public:
	explicit PlaylistManager(TabPlaylist *parent);

	int savePlaylist(Playlist *p, bool isOverwriting, bool isExiting);

public slots:
	void saveAndRemovePlaylist(Playlist *p, int index, bool isOverwriting = false, bool isExiting = false);

signals:
	void aboutToRemovePlaylist(int);
};

#endif // PLAYLISTMANAGER_H
