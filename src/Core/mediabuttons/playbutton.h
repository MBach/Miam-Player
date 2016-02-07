#ifndef PLAYBUTTON_H
#define PLAYBUTTON_H

#include "mediabutton.h"
#include "mediaplayer.h"

/**
 * \brief		The PlayButton class can change its icon automatically when the status of the player has changed.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY PlayButton : public MediaButton
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	explicit PlayButton(QWidget *parent = nullptr);

	void setMediaPlayer(MediaPlayer *mediaPlayer);
};

#endif // PLAYBUTTON_H
