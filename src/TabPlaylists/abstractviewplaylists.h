#ifndef ABSTRACTVIEWPLAYLISTS_H
#define ABSTRACTVIEWPLAYLISTS_H

#include <abstractview.h>
#include <mediaplayer.h>

#include "miamtabplaylists_global.hpp"

/**
 * \brief		The AbstractViewPlaylists class
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

	virtual int selectedTracksInCurrentPlaylist() const = 0;

public slots:
	virtual void addPlaylist() = 0;

	virtual void removeCurrentPlaylist() = 0;
};

#endif // ABSTRACTVIEWPLAYLISTS_H
