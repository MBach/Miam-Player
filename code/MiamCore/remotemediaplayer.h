#ifndef REMOTEMEDIAPLAYER_H
#define REMOTEMEDIAPLAYER_H

#include <QObject>
#include "miamcore_global.h"

class MIAMCORE_LIBRARY RemoteMediaPlayer : public QObject
{
	Q_OBJECT
public:
	explicit RemoteMediaPlayer(QObject *parent = 0);

	virtual ~RemoteMediaPlayer();
};

#endif // REMOTEMEDIAPLAYER_H
