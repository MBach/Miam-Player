#ifndef IMEDIAPLAYER_H
#define IMEDIAPLAYER_H

#include <QObject>
#include "miamcore_global.h"

/**
 * \brief		IMediaPlayer class is a pure virtual class implemented by plugins.
 * \details		When one wants to extend this player by enabling remote playing, it must use this interface.
 *				Signals are already connected to MediaPlayer class which acts as a dispatcher.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY IMediaPlayer : public QObject
{
	Q_OBJECT
public:
	explicit IMediaPlayer(QObject *parent = 0) : QObject(parent) {}

	virtual ~IMediaPlayer() {}

	/** The host acts as a key to play medias, like http://www.my-favourite-streaming-website.com/. */
	virtual QString host() const = 0;

	/** Current media length in ms. */
	virtual qint64 duration() const = 0;

	/** The position in the current media being played. Percent-based. */
	virtual qreal position() const = 0;

	virtual void setMute(bool b) = 0;

	/** Sets the current position in ms in the current media being played (useful for seeking). */
	virtual void setPosition(qint64 pos) = 0;

	/** Sets the total time in ms in the current media being played (useful for seeking). */
	virtual void setTime(qint64 t) = 0;
	virtual qint64 time() const = 0;

	/** The current volume of this remote player. */
	virtual qreal volume() const = 0;

public slots:
	/// Must be implemented in derived class
	virtual void pause() = 0;
	virtual void play(const QUrl &track) = 0;
	virtual void resume() = 0;
	virtual void seek(float pos) = 0;
	virtual void setVolume(qreal volume) = 0;
	virtual void stop() = 0;

signals:
	/** Current player has been put in pause mode by one. */
	void paused();

	/** Current track position has changed. 'Pos' and 'duration' are expressed in milliseconds. */
	void positionChanged(qint64 pos, qint64 duration);

	/** The player has started to play a new track. 'Duration' is expressed in milliseconds.*/
	void started(qint64 duration);

	/** Current player has been put in stop mode by one. */
	void stopped();

	/** Current track has finished to be played. */
	void trackHasEnded();
};

#endif // IMEDIAPLAYER_H
