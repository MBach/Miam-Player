#ifndef MEDIAPLAYERCONTROL_H
#define MEDIAPLAYERCONTROL_H

#include "miamcore_global.h"
#include "mediaplayer.h"

/**
 * \brief		The MediaPlayerControl class is a class that can control a MediaPlayer instance.
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaPlayerControl : public QObject
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	explicit MediaPlayerControl(MediaPlayer *mediaPlayer, QObject *parent = nullptr)
		: QObject(parent)
		, _mediaPlayer(mediaPlayer) {}

	virtual ~MediaPlayerControl() {}

	virtual bool isInShuffleState() const = 0;

	inline MediaPlayer* mediaPlayer() const { return _mediaPlayer; }

public slots:
	virtual void skipBackward() = 0;

	virtual void skipForward() = 0;

	virtual void stop() = 0;

	virtual void togglePlayback() = 0;

	virtual void toggleShuffle(bool checked) = 0;
};

#endif // MEDIAPLAYERCONTROL_H
