#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY MediaPlayer : public QMediaPlayer
{
	Q_OBJECT
public:
	explicit MediaPlayer(QObject *parent = 0);

public slots:
	/** Seek backward in the current playing track for a small amount of time. */
	void seekBackward();

	/** Seek forward in the current playing track for a small amount of time. */
	void seekForward();

	/** Change the current track. */
	void skipBackward();

	/** Change the current track. */
	void skipForward();
};

#endif // MEDIAPLAYER_H
