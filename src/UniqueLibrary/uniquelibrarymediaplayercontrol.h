#ifndef UNIQUELIBRARYMEDIAPLAYERCONTROL_H
#define UNIQUELIBRARYMEDIAPLAYERCONTROL_H

#include <mediaplayer.h>
#include <mediaplayercontrol.h>

#include "miamuniquelibrary_global.hpp"

class UniqueLibrary;

/**
 * \brief	The UniqueLibraryMediaPlayerControl class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryMediaPlayerControl : public MediaPlayerControl
{
	Q_OBJECT
private:
	UniqueLibrary *_uniqueLibrary;

public:
	explicit UniqueLibraryMediaPlayerControl(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

	virtual bool isInShuffleState() const override;

	inline void setUniqueLibrary(UniqueLibrary *uniqueLibrary) { _uniqueLibrary = uniqueLibrary; }

	virtual void skipBackward() override;

	virtual void skipForward() override;

	virtual void stop() override;

	virtual void togglePlayback() override;

	virtual void toggleShuffle(bool checked) override;
};

#endif // UNIQUELIBRARYMEDIAPLAYERCONTROL_H
