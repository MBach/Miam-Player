#ifndef ABSTRACTMEDIAPLAYERCONTROL_H
#define ABSTRACTMEDIAPLAYERCONTROL_H

#include "miamcore_global.h"
#include "mediaplayer.h"

/**
 * \brief		The AbstractMediaPlayerControl class is a class that can control a MediaPlayer instance.
 * \details		This abstract class must be implemented by every view. Each view has not the same features but they all have a commun interface
 *				to control the mediaPlayer instance. For example, ViewPlaylists class doesn't handle shuffle mode like UniqueLibrary class.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY AbstractMediaPlayerControl : public QObject
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	explicit AbstractMediaPlayerControl(MediaPlayer *mediaPlayer, QObject *parent = nullptr)
		: QObject(parent)
		, _mediaPlayer(mediaPlayer) {}

	virtual ~AbstractMediaPlayerControl() {}

	virtual bool isInShuffleState() const = 0;

	inline MediaPlayer* mediaPlayer() const { return _mediaPlayer; }

public slots:
	virtual void skipBackward() = 0;

	virtual void skipForward() = 0;

	virtual void stop() = 0;

	virtual void togglePlayback() = 0;

	virtual void toggleShuffle(bool checked) = 0;
};

#endif // ABSTRACTMEDIAPLAYERCONTROL_H
