#ifndef REMOTEMEDIAPLAYER_H
#define REMOTEMEDIAPLAYER_H

#include <QObject>
#include "miamcore_global.h"

/**
 * \brief		RemoteMediaPlayer class is a pure virtual class implemented by plugins.
 * \details		When one wants to extend this player by enabling remote playing, it must use this interface.
 *				Signals are already connected to MediaPlayer class which acts as a dispatcher.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY RemoteMediaPlayer : public QObject
{
	Q_OBJECT
public:
	explicit RemoteMediaPlayer(QObject *parent = 0) : QObject(parent) {}

	virtual ~RemoteMediaPlayer() {}

	virtual QString host() const = 0;

	virtual float position() const = 0;

	virtual void setTime(int t) = 0;

public slots:
	virtual void pause() = 0;
	virtual void play(const QUrl &track) = 0;
	virtual void resume() = 0;
	virtual void seek(float pos) = 0;
	virtual void setVolume(int volume) = 0;
	virtual void stop() = 0;

signals:
	void paused();

	/** Current track position has changed. 'Pos' and 'duration' are expressed in milliseconds. */
	void positionChanged(qint64 pos, qint64 duration);

	/** The player has started to play a new track. 'Duration' is expressed in milliseconds.*/
	void started(qint64 duration);

	void stopped();

	/** Current track has finished. */
	void trackHasEnded();
};

#endif // REMOTEMEDIAPLAYER_H
