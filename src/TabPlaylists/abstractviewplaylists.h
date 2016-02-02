#ifndef ABSTRACTVIEWPLAYLISTS_H
#define ABSTRACTVIEWPLAYLISTS_H

#include <abstractview.h>
#include <mediaplayer.h>
#include <QModelIndex>

#include "miamtabplaylists_global.hpp"

/**
 * \brief		The AbstractViewPlaylists class is the base class for views which can handle playlists in Miam-Player.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY AbstractViewPlaylists : public AbstractView
{
	Q_OBJECT

protected:
	MediaPlayer *_mediaPlayer;

public:
	AbstractViewPlaylists(MediaPlayer *mediaPlayer) : AbstractView(nullptr), _mediaPlayer(mediaPlayer) {}

	virtual ~AbstractViewPlaylists() {}

	virtual void addToPlaylist(const QList<QUrl> &tracks) = 0;

	/** Open a new Dialog where one can add a folder to current playlist. */
	virtual void openFolder(const QString &dir) const = 0;

	virtual void saveCurrentPlaylists() = 0;

	virtual int selectedTracksInCurrentPlaylist() const = 0;

public slots:
	virtual void addExtFolders(const QList<QDir> &) = 0;

	virtual void addPlaylist() = 0;

	virtual void moveTracksUp() = 0;

	virtual void moveTracksDown() = 0;

	virtual void openFiles() = 0;

	virtual void openFolderPopup() = 0;

	virtual void openPlaylistManager() = 0;

	virtual void removeCurrentPlaylist() = 0;

	virtual void removeSelectedTracks() = 0;

signals:
	void aboutToSendToTagEditor(const QModelIndexList &indexes, const QList<QUrl> &tracks);
};

#endif // ABSTRACTVIEWPLAYLISTS_H
