#include "mediaplaylist.h"

#include <algorithm>
#include <QtDebug>

MediaPlaylist::MediaPlaylist(QObject *parent)
	: QMediaPlaylist(parent)
{
	connect(this, &QMediaPlaylist::playbackModeChanged, this, [=](PlaybackMode mode) {
		if (mode == Random) {
			this->createRandom();
		} else {
			this->resetRandom();
		}
	});
}

void MediaPlaylist::createRandom()
{
	qDebug() << Q_FUNC_INFO;
	for (int i = 0; i < mediaCount(); i++) {
		_randomIndexes.push_back(i);
	}
	std::random_shuffle(_randomIndexes.begin(), _randomIndexes.end());
}

void MediaPlaylist::resetRandom()
{
	qDebug() << Q_FUNC_INFO;
	_randomIndexes.clear();
}

void MediaPlaylist::next()
{
	qDebug() << Q_FUNC_INFO;
	if (playbackMode() == Random) {
		qDebug() << Q_FUNC_INFO;
		for (int i = 0; i < mediaCount(); i++) {
			qDebug() << "index" << i << _randomIndexes[i];
		}
		qDebug() << "_randomIndexes 1" << _randomIndexes[currentIndex()];
		this->setCurrentIndex(_randomIndexes[QMediaPlaylist::currentIndex() + 1]);
		qDebug() << "_randomIndexes 2" << _randomIndexes[currentIndex()];
	} else {
		qDebug() << "currentIndex 1" << currentIndex();
		QMediaPlaylist::next();
		qDebug() << "currentIndex 2" << currentIndex();
	}
}

