#ifndef VIEWPLAYLISTSMEDIAPLAYERCONTROL_H
#define VIEWPLAYLISTSMEDIAPLAYERCONTROL_H

#include <mediaplayer.h>
#include <mediaplayercontrol.h>

/**
 * \brief		The ViewPlaylistsMediaPlayerControl class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ViewPlaylistsMediaPlayerControl : public MediaPlayerControl
{
	Q_OBJECT
public:
	explicit ViewPlaylistsMediaPlayerControl(MediaPlayer *mediaPlayer, QObject *parent = nullptr);

	virtual ~ViewPlaylistsMediaPlayerControl() {}

	/** Returns true if current playlist has a plabackMode to Random. */
	virtual bool isInShuffleState() const override;

	/** Forward action to MediaPlayer. */
	virtual void skipBackward() override;

	/** Forward action to MediaPlayer. */
	virtual void skipForward() override;

	/** Forward action to MediaPlayer. */
	virtual void stop() override;

	/** Forward action to MediaPlayer. */
	virtual void togglePlayback() override;

	virtual void toggleShuffle(bool checked) override;
};

#endif // VIEWPLAYLISTSMEDIAPLAYERCONTROL_H
